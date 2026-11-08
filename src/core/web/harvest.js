(() => {
    const out = {title: document.title.replace(/\s*[|—–]\s*(VK|ВКонтакте|Яндекс Музыка).*$/i, ''), tracks: [], url: location.href};
    const decode = value => { const node = document.createElement('textarea'); node.innerHTML = String(value || ''); return node.value.trim().normalize('NFC'); };
    const catalogSearch = Boolean(window.__auroraCatalogSearch);
    const recommendations = Boolean(window.__auroraRecommendations);
    const paged=typeof window!=='undefined'?window.__auroraVkLibrary:null;
    if(paged?.entryUrl)out.entryUrl=paged.entryUrl;
    const visible = node => Boolean(node && (!node.getClientRects || (node.getClientRects().length && getComputedStyle(node).visibility !== 'hidden')));
    const all = (root, selector) => Array.from(root.querySelectorAll(selector));
    const rowSelector = '[data-audio], [data-full-id], [data-audio-id], a[href*="/track/"]';
    // A hidden legacy placeholder or an unrelated <main> must not hide the new VK list.
    let scope = document;
    for (const selector of (catalogSearch ? ['main', '[role="main"]'] : ['[data-audio-context="my_audios_full"]', '.CatalogBlock__my_audios', '[data-testid="MusicPlaylistTracks_Items"]', '[data-testid="audiolistitems-list"]', '.audio_page__audio_rows_list', '.audio_page__audio_rows', '[data-test-id="playlist-tracks"]', '[data-testid="playlist-tracks"]', 'main'])) {
        const node = document.querySelector(selector);
        if (visible(node) && all(node, rowSelector).length) { scope = node; break; }
    }
    if (recommendations) {
        // Use the site's own recommendation navigation when an obsolete entry URL
        // restores the library tab. Never open a playlist or a track at random.
        window.__auroraRecommendationControls ||= new WeakSet();
        const target = all(document, 'a[href], [role="tab"], button').find(node => {
            if (!visible(node) || window.__auroraRecommendationControls.has(node)) return false;
            const label = (node.textContent || node.getAttribute('aria-label') || '').trim();
            if (!/^(для вас|рекомендации|вам понравится|for you|recommendations)$/i.test(label)) return false;
            if (node.getAttribute('aria-selected') === 'true' || node.getAttribute('aria-current')) return false;
            if (node.href) {
                const url = new URL(node.href, location.href);
                if (url.origin !== location.origin || !/audio|music/.test(url.pathname)) return false;
                if (url.href === location.href) return false;
            }
            return true;
        });
        if (target) {
            window.__auroraRecommendationControls.add(target);
            target.click();
            out.loading = true;
            return JSON.stringify(out);
        }
    }
    const seen = new Map();
    function add(track) {
        if (!track.title || !track.url) return;
        if (seen.has(track.url)) {
            const previous = seen.get(track.url);
            for (const [key, value] of Object.entries(track)) if (!previous[key] && value) previous[key] = value;
            return;
        }
        seen.set(track.url, track); out.tracks.push(track);
    }
    function audio(value) {
        if (Array.isArray(value)) {
            if (Array.isArray(value[0])) { value.forEach(audio); return; }
            if (value.length < 6 || !/^[1-9]\d*$/.test(String(value[0])) || !/^-?[1-9]\d*$/.test(String(value[1]))) return;
            const media = typeof value[2] === 'string' && /^https:\/\//i.test(value[2]) && !value[2].includes('audio_api_unavailable') ? value[2] : '';
            const covers=typeof value[14]==='string'?value[14].split(',').filter(url=>/^https:\/\//.test(url)):[];
            add({title: decode(value[3]), artist: decode(value[4]), durationMs: Math.round(Number(value[5]) * 1000),
                coverUrl: covers.at(-1) || '', album: typeof value[19]==='object'?value[19]?.title || '':'',
                url: new URL('/audio' + value[1] + '_' + value[0], location.origin).href, mediaUrl: media, rawAudio: value});
        } else if (value && typeof value === 'object' && value.id && value.owner_id && value.title) {
            audio([value.id, value.owner_id, value.url || '', value.title, value.artist || value.performer || '', value.duration || 0]);
        }
    }
    const legacy = all(scope, '[data-audio]');
    if(paged?.rows?.length){paged.rows.forEach(audio);out.expectedCount=paged.total || 0;}
    for (const node of legacy) { try { audio(JSON.parse(node.getAttribute('data-audio'))); } catch (_) {} }
    const modern = all(scope, '[data-audio-id]');
    for (const node of [...all(scope, '[data-full-id]'), ...modern]) {
        const identity = node.getAttribute('data-audio-id') || node.getAttribute('data-full-id') || '';
        const match = /^(-?\d+)_(\d+)(?:_|$)/.exec(identity); if (!match) continue;
        const title = node.querySelector('[data-testid="MusicTrackRow_Title"], .audio_row__title_inner, .audio_row__title, [class*="AudioRow__title"]');
        const artist = node.querySelector('[data-testid="MusicTrackRow_Authors"], .audio_row__performers, [class*="AudioRow__artist"], [class*="AudioRow__performer"]');
        const duration = node.querySelector('[data-testid="MusicTrackRow_Duration"], .audio_row__duration, time');
        const time = duration && /(?:(\d+):)?(\d+):(\d{2})/.exec(duration.textContent.trim());
        if (title && visible(node)) add({title: title.textContent.trim(), artist: artist ? artist.textContent.trim() : '',
            coverUrl: node.querySelector('img')?.src || '',
            durationMs: time ? ((Number(time[1] || 0) * 60 + Number(time[2])) * 60 + Number(time[3])) * 1000 : 0,
            url: new URL('/audio' + match[1] + '_' + match[2], location.origin).href, vkFullId: identity});
    }
    // Read the page's own playlist state too: virtualized rows do not represent the full list.
    let stateLists = 0, stateTracks = 0, stateLoading = Boolean(paged && !paged.failed && !paged.complete && paged.owner), modernModels = 0, fiberNodes = 0, playlistCandidates = 0, modelReadErrors = 0, modelDepth = 0;
    if (typeof window !== 'undefined' && /(^|\.)vk\.(ru|com)$/.test(location.hostname || new URL(location.href).hostname)) {
        const owner = /^\/audios(-?\d+)/.exec(location.pathname || new URL(location.href).pathname);
        const playlist = /\/music\/playlist\/(-?\d+)_(\d+)/.exec(location.pathname || '');
        const albumRoute = /\/music\/album\/(\d+)_(-?\d+)/.exec(location.pathname || '');

        const wantedOwner = catalogSearch?'':owner ? owner[1] : playlist ? playlist[1] : albumRoute ? albumRoute[2] : String(window.vk?.id || '');
        const wantedAlbum = playlist ? playlist[2] : albumRoute ? albumRoute[1] : null;
        // The current VK music app keeps its playlist in React props rather
        // than window.getAudioPlayer(). Read only the ancestors of a music row,
        // using the playlist's own normal pagination method. Do not traverse
        // the global application store or unrelated component state.
        const modelLists = new Set(), fibersSeen = new Set(), propsSeen = new Set();
        const catalogAnchors = catalogSearch
            ? all(scope, '[data-audio-id], [data-full-id], [data-audio], [data-testid*="Track"], [class*="AudioRow"], a[href*="/audio"]').filter(visible).slice(0, 100)
            : [];
        const anchors=[modern[0],modern[Math.floor(modern.length/2)],modern.at(-1),...catalogAnchors,scope];
        for (const anchor of anchors) for (let node=anchor,up=0; node && up<14; node=node.parentElement,up++) {
            const key=Object.getOwnPropertyNames(node).find(key=>key.startsWith('__reactFiber$') || key.startsWith('__reactInternalInstance$'));
            let fiber=key && node[key];
            for(let depth=0;fiber && depth<120;++depth,fiber=fiber.return) {
              if(fibersSeen.has(fiber))continue;
              fibersSeen.add(fiber);++fiberNodes;modelDepth=Math.max(modelDepth,depth);
              const choices=[];
              for(const version of [fiber,fiber.alternate]) {
                if(!version)continue;
                for(const props of [version.memoizedProps,version.pendingProps]) {
                  if(!props || typeof props!=='object')continue;
                  choices.push(props);
                  for(const holder of [props.value,props.model]) {
                    if(!holder || typeof holder!=='object')continue;
                    choices.push(holder);
                    try { if(holder.content)choices.push(holder.content);if(holder.props?.current)choices.push(holder.props.current); } catch(_) {++modelReadErrors;}
                  }
                }
                // VK's scoped dependency providers keep their component model
                // in a ref hook; it is not necessarily a prop on a DOM ancestor.
                for(let hook=version.memoizedState,n=0;hook && n<16;hook=hook.next,n++) {
                  const holder=hook.memoizedState?.current;
                  if(holder && typeof holder==='object') {choices.push(holder);try{if(holder.content)choices.push(holder.content);}catch(_){++modelReadErrors;}}
                }
              }
              // Current catalog wrappers expose the component's renderProps.
              // Follow only named music-model holders, with a small hard bound.
              for(let i=0;i<choices.length && i<48;++i) {
                const holder=choices[i];if(!holder || typeof holder!=='object')continue;
                for(const name of ['renderProps','model','tracksModel','content'])try {
                    const child=holder[name];if(child && typeof child==='object' && !choices.includes(child) && choices.length<48)choices.push(child);
                }catch(_){++modelReadErrors;}
              }
              for(const props of choices) {
              if(!props || typeof props!=='object' || propsSeen.has(props))continue;propsSeen.add(props);
              try {
                const model=props.tracksModel;
                if (catalogSearch) {
                    // Recommendation shelves also expose plain arrays and individual
                    // track props, without the library's infinite-playlist interface.
                    const readTrack = item => {
                        if (Array.isArray(item)) { audio(item); return; }
                        if (!item || typeof item !== 'object') return;
                        const data = item.data || item;
                        const identity = item.identity?.fullId || data.fullId ||
                            (data.owner_id && data.id ? `${data.owner_id}_${data.id}` : '');
                        if (!/^(-?\d+)_(\d+)$/.test(identity) || !data.title) return;
                        add({title: decode(data.title), artist: decode(item.authors?.single || data.artist || data.performer || ''),
                            durationMs: Number(data.durationMs) || Math.round(Number(data.duration || 0) * 1000),
                            coverUrl: item.cover?.pickByWidth?.(400)?.src || data.album?.thumb?.photo_300 || data.thumb?.photo_300 || '',
                            album: data.album?.title || '', url: new URL('/audio' + identity, location.origin).href,
                            vkFullId: identity, mediaUrl: typeof data.url === 'string' && /^https:\/\//.test(data.url) && !data.url.includes('audio_api_unavailable') ? data.url : ''});
                    };
                    for (const item of [props.track, props.audio, props.item, props.data]) readTrack(item);
                    for (const list of [props.tracks, props.audios, props.items, model?.tracks, props.list?.head, props.playlist?.head])
                        if (Array.isArray(list)) list.slice(0, 200).forEach(readTrack);
                }
                const list=model?.playlist || props.list || props.playlist;
                const tracks=model?.tracks || list?.head || props.tracks;
                const total=Number(model?.count ?? list?.count ?? props.count);
                if(list)++playlistCandidates;
                const identity=list?.identity || {};
                const identityOwner=identity.ownerId ?? identity.owner_id;
                if(wantedOwner && identityOwner!==undefined && String(identityOwner)!==wantedOwner)continue;
                const identityAlbum=identity.playlistId ?? identity.id;
                if(wantedAlbum!==null && identityAlbum!==undefined && String(identityAlbum)!==wantedAlbum)continue;
                if(!catalogSearch && wantedAlbum===null && identityAlbum!==undefined && !['-1','0'].includes(String(identityAlbum)))continue;
                if(list && !modelLists.has(list) && typeof list.fetchNext==='function' && typeof list.ensureItems==='function' && (!Number.isFinite(total) || total<1)) {
                    window.__auroraModernInitializations ||= new WeakMap();
                    if(!window.__auroraModernInitializations.has(list)) {
                        const task={pending:true};window.__auroraModernInitializations.set(list,task);
                        try {Promise.resolve(list.ensureItems()).then(()=>{task.pending=false;},()=>{task.pending=false;task.failed=true;});}
                        catch(_){task.pending=false;task.failed=true;++modelReadErrors;}
                    }
                    stateLoading ||= window.__auroraModernInitializations.get(list).pending;
                }
                if(list?.infinite===false && typeof list.ensureItems!=='function' && typeof model?.fetchRest!=='function')continue;
                if(!list || modelLists.has(list) || !Array.isArray(tracks) || !Number.isFinite(total) || total<1 || total>100000)continue;
                if(typeof list.fetchNext!=='function' && typeof model?.fetchRest!=='function')continue;
                modelLists.add(list);++modernModels;++stateLists;
                out.expectedCount=Math.max(out.expectedCount || 0,total);
                const before=out.tracks.length;
                for(const [position,track] of tracks.entries()) {
                    const data=track?.data || track;
                    const fullId=track?.identity?.fullId || (data?.owner_id && data?.id ? `${data.owner_id}_${data.id}` : '');
                    if(!/^(-?\d+)_(\d+)$/.test(fullId) || !data?.title)continue;
                    const cover=track.cover?.pickByWidth?.(400)?.src || data.album?.thumb?.photo_300 || data.thumb?.photo_300 || '';
                    add({title:decode(data.title),artist:track.authors?.single || data.artist || data.performer || '',
                        durationMs:data.durationMs || Math.round(Number(data.duration || 0)*1000),coverUrl:cover,
                        album:data.album?.title || '',url:new URL('/audio'+fullId,location.origin).href,vkFullId:fullId,playlistPosition:position,
                        mediaUrl:typeof data.url==='string' && /^https:\/\//.test(data.url) && !data.url.includes('audio_api_unavailable')?data.url:'',
                        mediaVerified:typeof data.url==='string' && /^https:\/\//.test(data.url) && !data.url.includes('audio_api_unavailable')});
                }
                stateTracks+=out.tracks.length-before;
                window.__auroraModernLoads ||= new WeakMap();
                let task=window.__auroraModernLoads.get(list);
                const fetching=Boolean(model?.hasFetching || list.isHeadFetching || list.hasFetchingItems || task?.pending);
                if(!catalogSearch && tracks.length<total && !fetching && !task?.failed) {
                    task={pending:true};window.__auroraModernLoads.set(list,task);
                    try {
                        const result=typeof list.fetchNext==='function'?list.fetchNext(Math.min(100,total-tracks.length)):model.fetchRest();
                        Promise.resolve(result).then(()=>{task.pending=false;},()=>{task.pending=false;task.failed=true;});
                    } catch (_) {task.pending=false;task.failed=true;}
                }
                stateLoading ||= fetching || Boolean(task?.pending);
                if(tracks.length>=total && !fetching && !task?.failed) {
                    const ordered=out.tracks.filter(track=>Number.isInteger(track.playlistPosition));
                    if(ordered.length===total){out.tracks=ordered.sort((a,b)=>a.playlistPosition-b.playlistPosition);out.playlistOrdered=true;out.complete=true;}
                }
              } catch (_) { ++modelReadErrors; }
              }
            }
        }
        const page = window.cur?.audioPage;
        const pageLists = page?._data?.playlists;
        let candidates = [page?._pagePlaylist, ...(Array.isArray(pageLists) ? pageLists : [])], canonical = null;
        try { if (typeof window.getAudioPlayer === 'function') {
            const player=window.getAudioPlayer();
            candidates.push(...(player?.getPlaylists?.() || []));
            // The visible catalog is a preview. Ask VK's own player for the
            // user's complete library, including entries never rendered in DOM.
            if(wantedOwner && typeof player?.getPlaylist==='function') {
                canonical=player.getPlaylist('playlist',Number(wantedOwner),Number(wantedAlbum ?? -1));candidates.push(canonical);
            }
        } } catch (_) {}
        const visited = new Set();
        for (const candidate of candidates) {
            if (!candidate || visited.has(candidate)) continue; visited.add(candidate);
            try {
                const id = String(candidate.getOwnerId?.() ?? candidate.ownerId ?? candidate.owner_id ?? '');
                const album = String(candidate.getPlaylistId?.() ?? candidate.getAlbumId?.() ?? candidate.id ?? '');
                const type = candidate.getType?.() ?? candidate.type;
                if (type && !['playlist', 'my', 'album'].includes(type)) continue;
                if (!wantedOwner || id !== wantedOwner || (wantedAlbum !== null ? album !== wantedAlbum : !['-1', '0'].includes(album) && type !== 'my')) continue;
                if(canonical && candidate!==canonical)continue;
                const list = candidate.getAudiosList?.() ?? candidate.list;
                if (!Array.isArray(list)) continue;
                ++stateLists;
                const count = out.tracks.length; list.forEach(audio); stateTracks += out.tracks.length - count;
                const total = Number(candidate.getTotalCount?.() ?? candidate.totalCount ?? candidate.total_count ?? 0);
                if (Number.isFinite(total) && total > 0 && total <= 100000) out.expectedCount = Math.max(out.expectedCount || 0, total);
                const hasMore=candidate.hasMore?.();
                window.__auroraPlaylistLoads ||= new WeakMap();
                let task = window.__auroraPlaylistLoads.get(candidate);
                const refresh=candidate===canonical && !task && typeof candidate.setForceReload==='function';
                if ((hasMore || refresh) && candidate.isFullyLoadable?.() && typeof candidate.loadAll === 'function') {
                    if (!task) {
                        task = {pending: true}; window.__auroraPlaylistLoads.set(candidate, task);
                        try {
                            if(refresh)candidate.setForceReload(true);
                            candidate.loadAll((_,failed) => { task.pending = false; task.failed=Boolean(failed); });
                        } catch (_) { task.pending = false;task.failed=true; }
                    }
                    stateLoading ||= task.pending;
                }
                stateLoading ||= Boolean(task?.pending);
                if(hasMore===false && !refresh && !task?.pending && !task?.failed && list.length>0 && list.length>=total)out.complete=true;
            } catch (_) {}
        }
    }
    // Expand the site's own preview before treating a stationary 45-row list
    // as the end. The control must belong to this music list, not another page card.
    if(modern.length && !out.complete && scope!==document && typeof window!=='undefined') {
        window.__auroraExpandedControls ||= new WeakSet();
        const button=all(scope,'button, [role="button"]').find(node=>visible(node) && !node.disabled &&
            !window.__auroraExpandedControls.has(node) && /^(показать|загрузить)\s+(все|всё|ещё)(?:\s+(треки|песни|музыку))?(?:\s*\(\d+\))?$/i.test((node.textContent || node.getAttribute('aria-label') || '').trim()));
        if(button){window.__auroraExpandedControls.add(button);button.click();stateLoading=true;}
    }
    for (const link of all(scope, 'a[href*="/track/"], a[href*="/album/"][href*="/track/"]')) {
        const url = new URL(link.href, location.origin); if (!/\/track\/\d+/.test(url.pathname)) continue;
        const row = link.closest('[data-test-id*="track"], [data-testid*="track"], [role="row"], .d-track, [class*="Track_root"], [class*="TrackListItem"], [class*="TrackCard_root"], [class*="HorizontalCard_root"], [data-item-index]') || link.parentElement;
        const artists = row ? all(row, 'a[href*="/artist/"]').map(a => a.textContent.trim()).filter(Boolean) : [];
        const duration = row && row.querySelector('[class*="duration"], [class*="Duration"], time');
        const time = duration && /^(\d+):(\d{2})$/.exec(duration.textContent.trim());
        const title = link.textContent.trim();
        if (title) add({title, artist: artists.join(', '), durationMs: time ? (Number(time[1]) * 60 + Number(time[2])) * 1000 : 0, url: url.origin + url.pathname});
    }
    let scroller = document.scrollingElement || document.documentElement;
    // Prefer the scrollable ancestor of the actual list rather than a sidebar.
    for (let node = modern[0]?.parentElement || legacy[0]?.parentElement || scope; node && node !== document; node = node.parentElement) {
        if (node.clientHeight > 150 && node.scrollHeight > node.clientHeight + 50 && /auto|scroll/.test(getComputedStyle(node).overflowY)) { scroller = node; break; }
    }
    const previous = scroller.scrollTop;
    scroller.scrollTop = Math.min(scroller.scrollTop + Math.max(scroller.clientHeight * 0.8, 500), scroller.scrollHeight);
    out.atBottom = scroller.scrollTop + scroller.clientHeight >= scroller.scrollHeight - 8;
    out.moved = scroller.scrollTop !== previous;
    const loader = document.querySelector('[aria-busy="true"], .audio_rows__loader, .CatalogBlock__autoListLoader, [data-test-id="loader"]');
    out.loading = stateLoading || visible(loader);
    if(paged?.complete) {
        out.tracks=[];seen.clear();paged.rows.forEach(audio);
        out.expectedCount=paged.total || out.tracks.length;out.complete=true;out.playlistOrdered=true;out.loading=false;
    }
    {
        const counter=document.querySelector('[data-testid="musicplayliststatistics-count"], [data-testid="MusicPlaylistTracks_Count"], .audio_page__count, .audio_page__title_count');
        const count=Number((counter?.textContent || '').replace(/[^0-9]/g,''));
        if(count>0 && count<=100000)out.expectedCount=Math.max(out.expectedCount || 0,count);
    }
    const loginFields = all(document, 'input[type="password"], input[name="login"], input[name="loginfmt"]').some(visible);
    const hostname = location.hostname || new URL(location.href).hostname, pathname = location.pathname || new URL(location.href).pathname;
    out.loginRequired = !out.tracks.length && (loginFields || hostname === 'vk.id' || /^(passport|oauth|login|id)\./.test(hostname) || /^\/(login|auth)(\/|$)/.test(pathname));
    const text = (document.body?.innerText || '').slice(0, 12000);
    out.challenge = !out.tracks.length && (/captcha|challenge|проверка безопасности|подтвердите, что вы|проверяем ваш браузер/i.test(document.title + ' ' + text) || visible(document.querySelector('iframe[src*="captcha"], [class*="Captcha"], [id*="captcha"]')));
    out.knownEmpty = visible(document.querySelector('[data-testid="MusicPlaylistTracks_EmptyMessage"], .audio_page__empty'));
    out.diagnostics = {version: 4, readyState: document.readyState || '', legacyRows: legacy.length,
        modernRows: modern.length, fullIdRows: all(scope, '[data-full-id]').length, stateLists, stateTracks,
        recognized: out.tracks.length, expectedCount: out.expectedCount || 0, frames: all(document, 'iframe').length,
        modernModels,fiberNodes,playlistCandidates,modelReadErrors,modelDepth,apiPages:paged?.pages || 0,apiTracks:paged?.rows?.length || 0,apiPending:Boolean(paged?.pending),apiFailed:Boolean(paged?.failed),apiCode:paged?.code || '',apiRetries:paged?.retries || 0,apiWaitMs:Math.max(0,(paged?.nextAt || 0)-Date.now()),scrollHeight:scroller.scrollHeight || 0,viewportHeight:scroller.clientHeight || 0,
        loginRequired: out.loginRequired, challenge: out.challenge, loading: out.loading, knownEmpty: out.knownEmpty};
    return JSON.stringify(out);
})()
