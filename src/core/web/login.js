(() => {
    const visible=node=>Boolean(node && node.getClientRects().length && getComputedStyle(node).visibility!=='hidden');
    const loginFields=Array.from(document.querySelectorAll('input[type="password"], input[name="login"], input[name="loginfmt"]')).some(visible);
    const authPage=location.hostname==='vk.id' || /^(passport|oauth|login|id)\./.test(location.hostname) || /^\/(login|auth|join)(\/|$)/.test(location.pathname);
    const profile=document.querySelector('#top_profile_link, [class*="TopNavBtn__profile"], [class*="UserID-Avatar"], [class*="UserID-Account"], [aria-label*="Профиль"], [aria-label*="Аккаунт"]');
    const tracks=document.querySelector('[data-full-id], a[href*="/track/"]');
    const user=typeof window.vk==='object' && Number(window.vk.id)>0;
    return JSON.stringify({authenticated:!loginFields && !authPage && Boolean(user || profile || tracks),loginRequired:loginFields || authPage});
})()
