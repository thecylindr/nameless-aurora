(() => {
    const options = __OPTIONS__;
    const result = {tracks: [], submitted: options.submitted};
    const visible = node => node && node.getClientRects().length && getComputedStyle(node).visibility !== 'hidden';
    const text = (document.body?.innerText || '').slice(0, 6000);
    const secure = value => {try {if(typeof value!=='string' || !value)return '';const u=new URL(value,location.origin);return u.protocol==='https:' && !u.username && !u.password?u.href:'';}catch(_){return '';}};
    function zaycevTracks(value) {
        const found=[],seen=new Set();let visited=0;
        function walk(node,key='',depth=0) {
            if(!node || typeof node!=='object' || depth>9 || ++visited>8000 || found.length>=12)return;
            const id=String(node.id ?? key);
            if(typeof node.track==='string' && node.track && /^[1-9][0-9]{0,17}$/.test(id) && !seen.has(id)) {
                let seconds=0;const d=String(node.duration || '');if(d.includes(':'))for(const part of d.split(':'))seconds=seconds*60+Number(part);else seconds=Number(d);
                found.push({title:node.track,artist:node.artistName || '',durationMs:Number.isFinite(seconds)?Math.max(0,seconds)*1000:0,
                    url:location.origin+'/pages/'+(id.length>2?id.slice(0,-2):'0')+'/'+id+'.shtml',coverUrl:secure(node.imageJpg || node.imageWebp),
                    ...(typeof node.download==='string' && node.download?{downloadHash:node.download}:{}),
                    ...(typeof node.streaming==='string' && node.streaming?{streamingHash:node.streaming}:{})});seen.add(id);return;
            }
            if(Array.isArray(node.trackIds) && node.tracksInfo)for(let i=0;i<node.trackIds.length && i<50;++i){const id=String(node.trackIds[i]),record=Array.isArray(node.tracksInfo)?node.tracksInfo.find(t=>String(t.id)===id) || node.tracksInfo[i]:node.tracksInfo[id];if(record)walk({...record,id},id,depth+1);}
            for(const [name,child] of Object.entries(node))if(!/auth|token|cookie|profile|account|password/i.test(name))walk(child,Array.isArray(node)?'':name,depth+1);
        }
        walk(value);return found;
    }
    function zaycevRequest(path,key) {
        if(typeof fetch!=='function' || typeof AbortController!=='function')return {pending:false};
        let state=window.__auroraZaycevRequest;
        if(!state || state.key!==key) {
            state=window.__auroraZaycevRequest={key,pending:true};
            const controller=new AbortController(),timer=setTimeout(()=>controller.abort(),8000);
            fetch(new URL(path,location.origin),{credentials:'include',headers:{Accept:'application/json'},signal:controller.signal})
                .then(async response=>{state.status=response.status;if(!response.ok)return;const body=await response.text();if(body.length<4194304)state.data=JSON.parse(body);})
                .catch(()=>{state.error=true;}).finally(()=>{clearTimeout(timer);state.pending=false;});
        }
        return state;
    }
    function siteTrack(node,id) {
        if(id===undefined || id===null)return null;
        // Current Hitmo TrackList passes a public item with play/download URLs.
        for(let element=node,up=0;element && up<5;element=element.parentElement,up++) {
            const key=Object.getOwnPropertyNames(element).find(key=>key.startsWith('__reactFiber$'));
            for(let fiber=key && element[key],depth=0;fiber && depth<20;fiber=fiber.return,depth++) {
                const p=fiber.memoizedProps,item=p?.item || p?.track;
                if(!item || String(item.id)!==String(id) || !item.title)continue;
                const media=secure(item.download || item.play || '');
                if(media)return {mediaUrl:media,mediaVerified:true,title:item.title,artist:item.artist || '',durationMs:Number(item.duration || 0)*1000,coverUrl:secure(item.image || '')};
            }
        }
        return null;
    }
    if (/captcha|подтвердите, что вы|проверка безопасности/i.test(document.title + ' ' + text))
        return JSON.stringify({blocked: true});
    if(options.source==='Zaycev.net') {
        let next=window.__NEXT_DATA__;
        if(!next){try{next=JSON.parse(document.querySelector('#__NEXT_DATA__')?.textContent || '{}');}catch(_){}}
        if(!options.resolve) {
            const tracks=zaycevTracks(next?.props);
            if(tracks.length)return JSON.stringify({submitted:true,tracks});
            const key='search:'+options.query,state=zaycevRequest('/api/external/pages/search/tracks?q='+encodeURIComponent(options.query)+'&page=1&limit=12',key);
            const apiTracks=zaycevTracks(state.data);
            if(apiTracks.length)return JSON.stringify({submitted:true,tracks:apiTracks});
            result.loading=state.pending;result.apiFinished=!state.pending;result.apiStatus=state.status || 0;
        } else {
            let candidate=options.candidate || {};
            if(!candidate.downloadHash && !candidate.streamingHash){const id=/\/pages\/\d+\/(\d+)\.shtml/.exec(location.pathname || '')?.[1];candidate=zaycevTracks(next?.props).find(track=>track.url.endsWith('/'+id+'.shtml')) || candidate;}
            const hash=candidate.downloadHash || candidate.streamingHash;
            if(typeof hash==='string' && hash) {
                const state=zaycevRequest('/api/external/track/'+(candidate.downloadHash?'download/':'play/')+encodeURIComponent(hash),'audio:'+hash);
                const data=state.data,media=secure(typeof data==='string'?data:data?.url || data?.data?.url || data?.data);
                if(media)return JSON.stringify({mediaUrl:media,mediaVerified:true});
                if(!state.pending && [401,403,423].includes(state.status))return JSON.stringify({accessDenied:true});
                if(state.pending)return JSON.stringify({loading:true});
            }
        }
    }
    if (options.resolve) {
        const targetId=/\/song\/(\d+)/.exec(location.pathname || '')?.[1];
        if(targetId)for(const node of [...document.querySelectorAll('h1, .track-item, button')].slice(0,80)) {
            const track=siteTrack(node,targetId);if(track)return JSON.stringify(track);
        }
        const media = [...document.querySelectorAll('audio, audio source')]
            .map(node => secure(node.currentSrc || node.src)).find(Boolean) || [...document.querySelectorAll('a[download]')].map(node=>secure(node.href)).find(url=>/\.(mp3|m4a|aac|ogg|opus|flac|wav|m3u8)(?:\?|$)/i.test(url));
        if (media) return JSON.stringify({mediaUrl: media,mediaVerified:true});
        for (const node of document.querySelectorAll('script[type="application/ld+json"]')) {
            try {
                const data = JSON.parse(node.textContent);
                const records = Array.isArray(data) ? data : [data, ...(data['@graph'] || [])];
                const record = records.find(item => /MusicRecording|AudioObject/.test(item?.['@type'] || '') && item.contentUrl);
                if (record) return JSON.stringify({mediaUrl: record.contentUrl});
            } catch (_) {}
        }
        if (!window.__auroraPublicPlayed) {
            const controls = [...document.querySelectorAll('button, [role="button"], a[download]')].filter(visible);
            const label = node => [node.textContent, node.getAttribute('aria-label'), node.getAttribute('title')].join(' ');
            const play = controls.find(node => /слушать|воспроизвести|\bplay\b/i.test(label(node)))
                || controls.find(node => /скачать\s*(mp3|трек|песню)?/i.test(label(node)));
            if (play) { window.__auroraPublicPlayed = true; play.click(); }
        }
        return JSON.stringify(result);
    }
    if (!options.submitted) {
        const input = [...document.querySelectorAll('input')].find(node => visible(node) &&
            /search|query|поиск/i.test([node.type, node.name, node.placeholder, node.getAttribute('aria-label')].join(' ')));
        if (!input) return JSON.stringify(result);
        const setter = Object.getOwnPropertyDescriptor(HTMLInputElement.prototype, 'value')?.set;
        if (setter) setter.call(input, options.query); else input.value = options.query;
        input.dispatchEvent(new Event('input', {bubbles: true}));
        input.dispatchEvent(new Event('change', {bubbles: true}));
        result.submitted = true;
        if (input.form?.requestSubmit) input.form.requestSubmit();
        else {
            input.dispatchEvent(new KeyboardEvent('keydown', {key: 'Enter', code: 'Enter', keyCode: 13, bubbles: true}));
            input.dispatchEvent(new KeyboardEvent('keyup', {key: 'Enter', code: 'Enter', keyCode: 13, bubbles: true}));
            const submit = (input.closest('form, [role="search"]') || input.parentElement)?.querySelector('button[type="submit"], button[aria-label*="Поиск"], button[title*="Поиск"]');
            if (submit) submit.click();
        }
        return JSON.stringify(result);
    }
    const seen = new Set();
    const queryWords = options.query.toLocaleLowerCase().split(/[^\p{L}\p{N}]+/u).filter(word => word.length > 1);
    for (const link of document.querySelectorAll('a[href]')) {
        const url = new URL(link.href, location.origin);
        if (url.origin !== location.origin || !(/^\/song\/\d+\/?$/.test(url.pathname) || /^\/pages\/\d+\/\d+\.shtml$/.test(url.pathname))) continue;
        const title = link.textContent.trim(); if (!title || seen.has(url.href) || !visible(link)) continue;
        let row = link.closest('.track-item, li');
        if (!row) {
            row = link.parentElement;
            for (let i = 0; i < 3 && row?.parentElement && row.parentElement.textContent.length < 500; i++) row = row.parentElement;
        }
        const label = (row?.textContent || title).trim();
        if (queryWords.length && !queryWords.some(word => label.toLocaleLowerCase().includes(word))) continue;
        const artist = row ? [...row.querySelectorAll('a[href]')].filter(node => node !== link && !/\/song\/|\/pages\//.test(node.href)).map(node => node.textContent.trim()).filter(Boolean).join(', ') : '';
        const time = /\b(\d{1,2}):(\d{2})\b/.exec(label);
        const track=siteTrack(row || link,/\/song\/(\d+)/.exec(url.pathname)?.[1]);
        result.tracks.push({url: url.origin + url.pathname, title, artist,
            durationMs: time ? (Number(time[1]) * 60 + Number(time[2])) * 1000 : 0,
            coverUrl: row?.querySelector('img')?.src || '',...(track || {})});
        seen.add(url.href); if (result.tracks.length === 12) break;
    }
    return JSON.stringify(result);
})()
