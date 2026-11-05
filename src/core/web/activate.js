(() => {
    const requested = __TRACK__;
    const result = activated => {
        const elements=Array.from(document.querySelectorAll('audio'));
        const element=elements.find(audio=>audio.currentSrc && !audio.paused) || elements.find(audio=>audio.currentSrc);
        return JSON.stringify({activated,mediaUrl:activated && element ? element.currentSrc : ''});
    };
    if(window.__auroraActivatedTrack===requested.url)return result(true);
    function click(button) {
        window.__auroraActivatedTrack=requested.url;button.click();return result(true);
    }
    const fullId = /\/audio(-?\d+_\d+)/.exec(requested.url || '');
    if (fullId) {
        const modern = Array.from(document.querySelectorAll('[data-audio-id]')).find(node => {
            const value = node.getAttribute('data-audio-id') || '';
            return value === fullId[1] || value.startsWith(fullId[1] + '_');
        });
        if (modern) {
            const control = modern.querySelector('[data-testid="MusicTrackRow_PlaybackControls"], [data-testid="MusicAlbumPlaylistTrackRow_PlaybackControls"]');
            return click(control || modern);
        }
        const row = document.querySelector('[data-full-id="'+fullId[1]+'"]');
        const play = row && row.querySelector('.audio_row__play_btn, button, [role="button"]');
        if (play) return click(play);
        if (requested.rawAudio && typeof window.getAudioPlayer === 'function') {
            try { window.__auroraActivatedTrack=requested.url;window.getAudioPlayer().play(requested.rawAudio); return result(true); }
            catch (_) { window.__auroraActivatedTrack=''; }
        }
        // A generic play button may belong to the first row or a recommendation,
        // not the requested track outside the initial preview. Never attach it
        // under another track's metadata.
        return result(false);
    }
    const play = document.querySelector('[data-test-id="play-button"], [data-testid="play-button"], button[aria-label*="Слушать"], button[aria-label*="Воспроизвести"], .button-play, .audio_row__play_btn');
    if (play) return click(play);
    const element = document.querySelector('audio');
    if (element) { window.__auroraActivatedTrack=requested.url;element.play().catch(()=>{});return result(true); }
    return result(false);
})()
