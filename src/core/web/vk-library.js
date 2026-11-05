(() => {
    if (!/(^|\.)vk\.(ru|com)$/.test(location.hostname)) return;
    const url=new URL(location.href), ownRoute=/^\/audios(-?\d+)\/?$/.exec(url.pathname);
    const playlist=/^\/music\/playlist\/(-?\d+)_(\d+)/.exec(url.pathname);
    const album=/^\/music\/album\/(\d+)_(-?\d+)/.exec(url.pathname);
    const generic=/^\/audio\/?$/.test(url.pathname);
    if(!generic && !ownRoute && !playlist && !album)return;
    const ownId=String(window.vk?.id || '');
    let owner=ownRoute?.[1] || playlist?.[1] || album?.[2] || (/^[1-9]\d*$/.test(ownId)?ownId:'');
    let listId=playlist?.[2] || album?.[1] || '-1';
    // Follow the account's own navigation, never a recommendation's user link.
    if(!owner && generic) {
        for(const selector of ['#l_aud a[href]', 'a#l_aud[href]', '#top_profile_link', '[data-testid="top-profile-link"]']) {
            const node=document.querySelector(selector);if(!node?.href)continue;
            const u=new URL(node.href,location.origin);if(u.origin!==location.origin)continue;
            const match=/^\/(?:audios|id)([1-9]\d*)\/?$/.exec(u.pathname);
            if(match){owner=match[1];break;}
        }
    }
    const key=owner+'_'+listId;
    let state=window.__auroraVkLibrary;
    if(!state || state.key!==key)state=window.__auroraVkLibrary={key,owner,listId,rows:[],seen:new Set(),offset:0,total:0,pages:0,pending:false,complete:false,failed:false,code:'idle',nextAt:0,retries:0};
    state.entryUrl=generic && /^[1-9]\d*$/.test(owner)?location.origin+'/audios'+owner+'?section=all':'';
    if(!owner || state.pending || state.complete || state.failed || Date.now()<state.nextAt)return;
    if(state.rows.length>=10000 || state.pages>=512){state.failed=true;state.code='size-limit';return;}
    const validTuple=t=>Array.isArray(t) && t.length>=6 && /^[1-9]\d*$/.test(String(t[0])) && /^-?[1-9]\d*$/.test(String(t[1])) && typeof t[3]==='string';
    function unwrap(value,depth=0) {
        if(depth>5 || value===null || value===undefined)return null;
        if(typeof value==='object' && !Array.isArray(value) && Array.isArray(value.list))return value;
        if(Array.isArray(value)) {
            if(typeof value[0]==='number' && value.slice(1).some(validTuple))return {totalCount:value[0],list:value.slice(1)};
            for(const child of value.slice(0,8)){const found=unwrap(child,depth+1);if(found)return found;}
        } else if(typeof value==='object')for(const k of ['payload','response','playlist','data']){const found=unwrap(value[k],depth+1);if(found)return found;}
        return null;
    }
    function fail(code,retry=false) {
        state.pending=false;state.code=code;
        if(retry && state.retries++<3){state.nextAt=Date.now()+(code==='rate-limited'?Math.min(30000,5000*2**(state.retries-1)):1500);return;}
        state.failed=true;
    }
    function consume(payload) {
        state.pending=false;
        const page=unwrap(payload);
        if(!page){fail('unsupported');return;}
        if(page.ownerId!==undefined && String(page.ownerId)!==owner){fail('wrong-owner');return;}
        if(page.id!==undefined && String(page.id)!==listId){fail('wrong-playlist');return;}
        const rows=page.list.filter(validTuple),before=state.rows.length;
        for(const row of rows){const id=row[1]+'_'+row[0];if(!state.seen.has(id)){state.seen.add(id);state.rows.push(row);}}
        const total=Number(page.totalCount ?? page.total_count ?? page.count);
        if(Number.isFinite(total) && total>0 && total<=100000)state.total=Math.max(state.total,total);
        state.pages++;state.retries=0;
        const hasCursor=Object.prototype.hasOwnProperty.call(page,'nextOffset');
        const end=hasCursor && page.nextOffset==='' || page.hasMore===false;
        if((state.total>0 && state.rows.length>=state.total) || (end && state.rows.length && (!state.total || state.rows.length>=state.total))){state.complete=true;state.code='complete';return;}
        if(end || !rows.length || state.rows.length===before){fail('incomplete');return;}
        const next=hasCursor?page.nextOffset:Number(state.offset)+rows.length;
        if(String(next)===String(state.offset)){fail('stalled');return;}
        state.offset=next;state.nextAt=Date.now()+600;state.code='loading';
    }
    const params={act:'load_section',type:'playlist',owner_id:owner,playlist_id:listId,offset:state.offset,al:1,is_loading_all:1};
    const access=url.searchParams.get('access_hash') || url.searchParams.get('access_key');if(access)params.access_hash=access;
    // Reuse a hash only from the requested playlist's own state, when exposed.
    try {
        const player=window.getAudioPlayer?.(), list=player?.getPlaylist?.('playlist',Number(owner),Number(listId));
        const hash=list?.getAccessHash?.();if(typeof hash==='string' && hash.length<2048)params.access_hash=hash;
        const context=list?.getContext?.();if(typeof context==='string' && context.length<2048)params.context=context;
        const from=list?.getFromId?.();if(Number.isFinite(from))params.from_id=from;
    }catch(_){}
    state.pending=true;state.code='loading';
    // Some current VK bundles expose ajax.post but never dispatch its callback.
    // Fall back within this page to one credentialled fetch of the same cursor.
    let completed=false,directStarted=false;
    const done=payload=>{if(completed)return;completed=true;consume(payload);};
    const reject=(code,retry=false)=>{if(completed)return;completed=true;fail(code,retry);};
    function direct() {
        if(completed || directStarted)return;directStarted=true;
        const controller=new AbortController(),timeout=setTimeout(()=>controller.abort(),22000);
        fetch(new URL('/al_audio.php',location.origin),{method:'POST',credentials:'include',signal:controller.signal,
            headers:{'Content-Type':'application/x-www-form-urlencoded','X-Requested-With':'XMLHttpRequest'},body:new URLSearchParams(params).toString()})
            .then(async response=>{
                if(completed)return;
                if(!response.ok){reject(response.status===429?'rate-limited':'denied',response.status===429 || response.status>=500);return;}
                const raw=await response.arrayBuffer();if(completed)return;
                // fetch.text() always assumes UTF-8; VK's legacy endpoint may be CP1251.
                const charset=/charset\s*=\s*["']?([^;\s"']+)/i.exec(response.headers.get('content-type') || '')?.[1];
                let text;
                try{text=new TextDecoder(charset || 'utf-8',{fatal:true}).decode(raw);}
                catch(_){text=new TextDecoder('windows-1251').decode(raw);}
                if(text.length>8388608){reject('response-size');return;}
                const start=text.indexOf('{');if(start<0){reject('unsupported');return;}
                let payload;try{payload=JSON.parse(text.slice(start));}catch(_){reject('unsupported');return;}
                if(Array.isArray(payload.payload) && Number(payload.payload[0])!==0){
                    const code=Number(payload.payload[0]);reject([6,9,29].includes(code)?'rate-limited':'denied',[6,9,29].includes(code));return;
                }
                done(payload);
            }).catch(()=>reject(controller.signal.aborted?'timeout':'network',true)).finally(()=>clearTimeout(timeout));
    }
    if(window.ajax && typeof window.ajax.post==='function') {
        const fallback=setTimeout(direct,3000);
        try{window.ajax.post('al_audio.php',params,{
            onDone:page=>{clearTimeout(fallback);done(page);},
            onFail:()=>{clearTimeout(fallback);direct();return true;}
        });}catch(_){clearTimeout(fallback);direct();}
    } else direct();
})()
