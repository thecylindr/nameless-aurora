import fs from 'node:fs';
import vm from 'node:vm';
import assert from 'node:assert/strict';
const script=fs.readFileSync(new URL('../src/core/web/harvest.js',import.meta.url),'utf8');
function run({vk=[],modern=[],links=[],login=false,loading=false,scrollTop=0,scrollHeight=1800,emptyLegacy=false,musicScope=null,window={},href='https://vk.ru/audios99',body='',captcha=null}={}) {
    const scroller={scrollTop,scrollHeight,clientHeight:600};
    const document={title:'Ночная коллекция | VK',readyState:'complete',body:{innerText:body},scrollingElement:scroller,documentElement:scroller,
        createElement(){return {set innerHTML(value){this.value=value.replaceAll('&amp;','&').replaceAll('&quot;','"');}};},
        querySelectorAll(selector){
            if(selector.startsWith('input'))return login?[{getClientRects(){return [{}];}}]:[];
            if(selector==='[data-audio]')return vk.map(value=>({getAttribute(){return JSON.stringify(value);}}));
            if(selector==='[data-audio-id]')return modern;
            if(selector.includes('a[href*="/track/" ]'))throw Error('Incorrect selector');
            if(selector.startsWith('a[href*="/track/"],'))return links;
            return [];
        },
        querySelector(selector){
            if(selector==='[data-testid="MusicPlaylistTracks_Items"]' && musicScope)return musicScope;
            if(emptyLegacy && (selector.startsWith('.audio_page') || selector==='main'))return {getClientRects(){return []},querySelectorAll(){return []}};
            if(selector.startsWith('iframe[src*="captcha"'))return captcha;
            return (selector.startsWith('[aria-busy') && loading) || (selector.startsWith('input') && login) ? {} : null;
        }
    };
    const url=new URL(href);
    const output=JSON.parse(vm.runInNewContext(script,{document,window,location:{href,origin:url.origin,hostname:url.hostname,pathname:url.pathname},URL,getComputedStyle(){return {overflowY:'auto',visibility:'visible'};}}));
    return {output,scroller};
}
const first=run({vk:[[10,99,'https://cdn.example/song.mp3','Ночь &amp; свет','МЫ',180],[11,-20,'audio_api_unavailable','Утро','Артист',205],[10,99,'','Ночь','МЫ',180]]});
assert.equal(first.output.tracks.length,2);assert.equal(first.output.tracks[0].url,'https://vk.ru/audio99_10');assert.equal(first.output.tracks[1].url,'https://vk.ru/audio-20_11');
assert.equal(first.output.tracks[0].title,'Ночь & свет');assert.equal(first.output.tracks[0].durationMs,180000);assert.equal(first.output.tracks[1].mediaUrl,'');
assert.equal(first.scroller.scrollTop,500);assert.equal(first.output.atBottom,false);
assert.equal(run({scrollTop:1200}).output.atBottom,true);assert.equal(run({login:true}).output.loginRequired,true);assert.equal(run({loading:true}).output.loading,true);
const nested=run({vk:[[[10,99,'','Первый','Автор',180],[11,99,'','Второй','Автор',200]]]});assert.equal(nested.output.tracks.length,2);
const row={querySelectorAll(){return [{textContent:'Артист 1'},{textContent:'Артист 2'}];},querySelector(){return {textContent:'3:04'};}};
const yandex=run({links:[{href:'https://music.yandex.ru/album/1/track/123?from=collection',textContent:'Песня',closest(){return row;}}]});
assert.equal(yandex.output.tracks[0].url,'https://music.yandex.ru/album/1/track/123');assert.equal(yandex.output.tracks[0].artist,'Артист 1, Артист 2');assert.equal(yandex.output.tracks[0].durationMs,184000);
const activate=fs.readFileSync(new URL('../src/core/web/activate.js',import.meta.url),'utf8').replace('__TRACK__',JSON.stringify({url:'https://vk.ru/audio99_10'}));
let clicks=0;const audio={currentSrc:'https://cdn.example/song.mp3',paused:false};
const context=vm.createContext({window:{},document:{querySelectorAll(selector){return selector==='audio'?[audio]:[];},querySelector(selector){return selector.startsWith('[data-full-id') ? {querySelector(){return {click(){clicks++;}};}}:null;}}});
assert.equal(JSON.parse(vm.runInContext(activate,context)).mediaUrl,audio.currentSrc);vm.runInContext(activate,context);assert.equal(clicks,1);

const loginScript=fs.readFileSync(new URL('../src/core/web/login.js',import.meta.url),'utf8');
function loginState(blocked,user) {
 return JSON.parse(vm.runInNewContext(loginScript,{location:{hostname:'vk.ru',pathname:'/audios99'},window:{vk:{id:user}},getComputedStyle(){return {visibility:'visible'}},document:{querySelector(){return null},querySelectorAll(){return blocked?[{getClientRects(){return [{}]}}]:[]}}}));
}
assert.equal(loginState(false,99).authenticated,true);assert.equal(loginState(true,99).authenticated,false);assert.equal(loginState(false,0).authenticated,false);

// Contract observed in VK's currently cached MusicTrackRow renderer, not legacy audio_row markup.
function modernRow(identity='99_10_access',title='Новый список',artist='Исполнитель',duration='3:04') {
    return {getAttribute(name){return name==='data-audio-id'?identity:null;},getClientRects(){return [{}]},
        querySelector(selector){
            if(selector.includes('MusicTrackRow_Title'))return {textContent:title};
            if(selector.includes('MusicTrackRow_Authors'))return {textContent:artist};
            if(selector.includes('MusicTrackRow_Duration'))return {textContent:duration};
            return null;
        }};
}
const modernOnly=run({modern:[modernRow()],emptyLegacy:true});
assert.equal(modernOnly.output.tracks.length,1);assert.equal(modernOnly.output.tracks[0].url,'https://vk.ru/audio99_10');
assert.equal(modernOnly.output.tracks[0].title,'Новый список');assert.equal(modernOnly.output.tracks[0].artist,'Исполнитель');
assert.equal(modernOnly.output.tracks[0].durationMs,184000);assert.equal(modernOnly.output.diagnostics.modernRows,1);
const longTrack=run({modern:[modernRow('99_11','Длинный трек','Автор','1:02:03')]});assert.equal(longTrack.output.tracks[0].durationMs,3723000);
const objectTrack=run({vk:[{id:10,owner_id:99,title:'Объект',performer:'Автор',duration:90}]});assert.equal(objectTrack.output.tracks[0].title,'Объект');
let modernClicks=0;
const modernControl={click(){modernClicks++}};
const rowToPlay=modernRow();rowToPlay.querySelector=()=>modernControl;
const modernContext=vm.createContext({window:{},document:{querySelectorAll(selector){return selector==='audio'?[audio]:selector==='[data-audio-id]'?[rowToPlay]:[];},querySelector(){throw Error('Modern playback must use the selected row')}}});
assert.equal(JSON.parse(vm.runInContext(activate,modernContext)).activated,true);assert.equal(modernClicks,1);
const absentContext=vm.createContext({window:{},document:{querySelectorAll(){return []},querySelector(selector){return selector.startsWith('[data-full-id')?null:{click(){throw Error('Wrong track must never be played')}}}}});
assert.equal(JSON.parse(vm.runInContext(activate,absentContext)).activated,false);

const list=[[10,99,'','Из состояния','Автор',180]];let loads=0,done;
const playlist={getOwnerId(){return 99},getPlaylistId(){return -1},getType(){return 'playlist'},getAudiosList(){return list},getTotalCount(){return 2},hasMore(){return list.length<2},isFullyLoadable(){return true},loadAll(callback){loads++;done=callback}};
const other={...playlist,getOwnerId(){return 100},getAudiosList(){return [[99,100,'','Чужая коллекция','Автор',20]]}};
const recommendation={...playlist,getType(){return 'recoms'},getAudiosList(){return [[98,99,'','Рекомендация','Автор',20]]}};
const siteWindow={cur:{audioPage:{_pagePlaylist:playlist}},getAudioPlayer(){return {getPlaylists(){return [playlist,other,recommendation]}}}};
const stateFirst=run({window:siteWindow});assert.equal(stateFirst.output.tracks.length,1);assert.equal(stateFirst.output.expectedCount,2);assert.equal(stateFirst.output.loading,true);assert.equal(loads,1);
list.push([11,99,'','Подгруженный трек','Автор',200]);done();
const stateComplete=run({window:siteWindow});assert.equal(stateComplete.output.tracks.length,2);assert.equal(stateComplete.output.loading,false);assert.equal(stateComplete.output.diagnostics.stateLists,1);assert.equal(loads,1);
assert.equal(run({body:'Подтвердите, что вы человек'}).output.challenge,true);
assert.equal(run({captcha:{getClientRects(){return []}}}).output.challenge,false);
assert.equal(run().output.knownEmpty,false);

// VK's player initially knows neither total count nor all 1,300 entries.
const fullLibrary=[[1,99,'','First','Artist',100]];let fullLoaded=false,requestedLibrary=[];
const completeLibrary={getOwnerId(){return 99},getPlaylistId(){return -1},getType(){return 'playlist'},getAudiosList(){return fullLibrary},
    getTotalCount(){return fullLoaded?1300:0},hasMore(){return !fullLoaded},isFullyLoadable(){return true},
    loadAll(done){for(let i=2;i<=1300;i++)fullLibrary.push([i,99,'','Track '+i,'Artist',120]);fullLoaded=true;done();}};
const libraryWindow={vk:{id:99},getAudioPlayer(){return {getPlaylists(){return []},getPlaylist(...args){requestedLibrary=args;return completeLibrary}}}};
run({window:libraryWindow});const all1300=run({window:libraryWindow});
assert.deepEqual(requestedLibrary,['playlist',99,-1]);assert.equal(all1300.output.expectedCount,1300);assert.equal(all1300.output.tracks.length,1300);assert.equal(all1300.output.complete,true);
const staleRows=Array.from({length:45},(_,i)=>[i+1,99,'','Track '+i,'Artist',120]);let forced=0;
const stalePreview={...completeLibrary,getAudiosList(){return staleRows},getTotalCount(){return staleRows.length},hasMore(){return false},
    setForceReload(value){if(value)forced++},loadAll(done){while(staleRows.length<1300)staleRows.push([staleRows.length+1,99,'','Full track','Artist',120]);done();}};
const staleWindow={vk:{id:99},getAudioPlayer(){return {getPlaylists(){return []},getPlaylist(){return stalePreview}}}};
run({window:staleWindow});const fresh1300=run({window:staleWindow});assert.equal(forced,1);assert.equal(fresh1300.output.tracks.length,1300);assert.equal(fresh1300.output.complete,true);
const coverRaw=[5,99,'','Cover','Artist',120];coverRaw[14]='https://cdn.example/small.jpg,https://cdn.example/large.jpg';coverRaw[19]={title:'Album'};
const coverTrack=run({vk:[coverRaw]}).output.tracks[0];assert.equal(coverTrack.coverUrl,'https://cdn.example/large.jpg');assert.equal(coverTrack.album,'Album');

// Contract recovered from the user's cached current VK client: React music
// rows receive a playlist with count/head/fetchNext; no global player exists.
const reactTracks=Array.from({length:27},(_,i)=>({identity:{fullId:`99_${i+1}`},data:{title:`React track ${i+1}`,artist:'Artist',duration:120},cover:{pickByWidth(){return {src:'https://cdn.example/cover.jpg'}}}}));
let reactFetches=0;
const reactList={identity:{ownerId:99,id:-1},count:1300,head:reactTracks,async fetchNext(count){
    reactFetches++;assert.ok(count<=100);
    for(let n=0;n<count && reactTracks.length<1300;n++)reactTracks.push({identity:{fullId:`99_${reactTracks.length+1}`},data:{title:`React track ${reactTracks.length+1}`,artist:'Artist',duration:120}});
}};
const reactRow=modernRow();reactRow['__reactFiber$fixture']={memoizedProps:{list:reactList},return:null};
const reactWindow={};let reactResult;
for(let n=0;n<70;n++){reactResult=run({modern:[reactRow],window:reactWindow}).output;await new Promise(resolve=>setImmediate(resolve));}
assert.equal(reactResult.expectedCount,1300);assert.equal(reactResult.tracks.length,1300);assert.equal(reactResult.complete,true);
assert.equal(reactResult.playlistOrdered,true);assert.equal(reactResult.tracks[0].url,'https://vk.ru/audio99_1');assert.equal(reactResult.tracks[1299].url,'https://vk.ru/audio99_1300');
assert.equal(reactResult.diagnostics.modernModels,1);assert.ok(reactFetches>=13);assert.equal(reactResult.tracks.find(t=>t.url.endsWith('99_1')).coverUrl,'https://cdn.example/cover.jpg');
const wrongRow=modernRow();wrongRow['__reactFiber$fixture']={memoizedProps:{list:{...reactList,identity:{ownerId:100,id:-1}}}};
assert.equal(run({modern:[wrongRow]}).output.diagnostics.modernModels,0);
const disposed=modernRow();disposed['__reactFiber$fixture']={memoizedProps:{get tracksModel(){throw Error('disposed model')}},return:{memoizedProps:{list:reactList}}};
assert.equal(run({modern:[disposed]}).output.tracks.length,1300);
let deepFiber={memoizedProps:{value:{content:{playlist:reactList}}}};
for(let depth=0;depth<65;++depth)deepFiber={memoizedProps:{},return:deepFiber};
const deepRow=modernRow();Object.defineProperty(deepRow,'__reactFiber$hidden',{value:deepFiber});
const deepModel=run({modern:[deepRow]}).output;assert.equal(deepModel.tracks.length,1300);assert.ok(deepModel.diagnostics.modelDepth>35);
const refRow=modernRow();refRow['__reactFiber$ref']={memoizedProps:{},memoizedState:{memoizedState:{current:{content:{playlist:reactList}}}}};
assert.equal(run({modern:[refRow]}).output.tracks.length,1300);
let initialized=0,finishInitialization;
const lazyList={...reactList,count:undefined,head:[],ensureItems(){initialized++;return new Promise(resolve=>{finishInitialization=()=>{lazyList.count=1300;lazyList.head=reactTracks;resolve();};});}};
const lazyRow=modernRow();lazyRow['__reactFiber$lazy']={memoizedProps:{playlist:lazyList}};
const lazyWindow={};assert.equal(run({modern:[lazyRow],window:lazyWindow}).output.loading,true);
assert.equal(run({modern:[lazyRow],window:lazyWindow}).output.loading,true);assert.equal(initialized,1);
finishInitialization();await new Promise(resolve=>setImmediate(resolve));
assert.equal(run({modern:[lazyRow],window:lazyWindow}).output.complete,true);
let wrongInitializations=0;
const unrelatedLazy=modernRow();unrelatedLazy['__reactFiber$wrong']={memoizedProps:{playlist:{...lazyList,count:undefined,identity:{ownerId:100},ensureItems(){wrongInitializations++;}}}};
run({modern:[unrelatedLazy]});assert.equal(wrongInitializations,0);
const nestedViewport={clientHeight:300,scrollHeight:5000,scrollTop:0};const nestedRow=modernRow();nestedRow.parentElement=nestedViewport;
const nestedScroll=run({modern:[nestedRow]});assert.equal(nestedViewport.scrollTop,500);assert.equal(nestedScroll.scroller.scrollTop,0);
let expansions=0;const expandButton={textContent:'Показать все',getClientRects(){return [{}]},click(){expansions++}};
const expandableScope={getClientRects(){return [{}]},querySelectorAll(selector){if(selector.startsWith('button'))return [expandButton];if(selector.includes('[data-audio-id]'))return [modernRow()];return []}};
const expandWindow={};assert.equal(run({musicScope:expandableScope,window:expandWindow}).output.loading,true);
run({musicScope:expandableScope,window:expandWindow});assert.equal(expansions,1);

const publicScript=fs.readFileSync(new URL('../src/core/web/public-source.js',import.meta.url),'utf8');
const publicRow={textContent:'Artist Song 3:04',querySelectorAll(){return [{href:'https://rus.hitmos.fm/artist/1',textContent:'Artist'}]},querySelector(){return {src:'https://cdn.example/cover.jpg'}}};
const publicLink={href:'https://rus.hitmos.fm/song/123',textContent:'Song',getClientRects(){return [{}]},closest(){return publicRow}};
function publicRun(options,document){return JSON.parse(vm.runInNewContext(publicScript.replace('__OPTIONS__',JSON.stringify(options)),{window:{},document,location:{origin:'https://rus.hitmos.fm'},URL,getComputedStyle(){return {visibility:'visible'}}}));}
const publicDocument={title:'Search',body:{innerText:''},querySelectorAll(selector){return selector==='a[href]'?[publicLink,{...publicLink,href:'https://evil.test/song/123'}]:[]}};
const publicResult=publicRun({query:'Artist Song',submitted:true,resolve:false},publicDocument);
assert.equal(publicResult.tracks.length,1);assert.equal(publicResult.tracks[0].durationMs,184000);assert.equal(publicResult.tracks[0].artist,'Artist');
publicRow['__reactFiber$fixture']={memoizedProps:{item:{id:123,title:'Song',artist:'Artist',duration:184,play:'https://cdn.example/stream?id=123',download:'https://cdn.example/file?id=123',image:'https://cdn.example/image.jpg'}}};
const publicCurrent=publicRun({query:'Artist Song',submitted:true,resolve:false},publicDocument);
assert.equal(publicCurrent.tracks[0].mediaUrl,'https://cdn.example/file?id=123');assert.equal(publicCurrent.tracks[0].mediaVerified,true);
const publicMedia=publicRun({resolve:true},{title:'Song',body:{innerText:''},querySelectorAll(){return [{currentSrc:'https://cdn.example/song.mp3'}]}});
assert.equal(publicMedia.mediaUrl,'https://cdn.example/song.mp3');
assert.equal(publicRun({resolve:false},{title:'CAPTCHA',body:{innerText:''}}).blocked,true);
const zaycevLink={...publicLink,href:'https://zaycev.net/pages/123/456.shtml'};
const zaycevDocument={...publicDocument,querySelectorAll(selector){return selector==='a[href]'?[zaycevLink]:[]}};
const zaycevResult=JSON.parse(vm.runInNewContext(publicScript.replace('__OPTIONS__',JSON.stringify({query:'Artist Song',submitted:true,resolve:false,source:'Zaycev.net'})),{window:{},document:zaycevDocument,location:{origin:'https://zaycev.net'},URL,getComputedStyle(){return {visibility:'visible'}}}));
assert.equal(zaycevResult.tracks[0].url,'https://zaycev.net/pages/123/456.shtml');
console.log('Additional contracts: complete 1300-track VK library, cover/album metadata, public-source candidates, media and challenge handling passed.');

console.log('Web scripts: legacy/modern VK rows, hidden placeholder, metadata/durations, playlist state/full loading and scope, modern playback, captcha diagnostics, Yandex and login passed.');
