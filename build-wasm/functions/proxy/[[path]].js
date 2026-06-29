// Cloudflare Pages Function: CORS proxy for powdertoy.co.uk
// Mirrors the Python proxy in serve.py for local dev.
// Mounted at /proxy/* — the game's fetch() interceptor rewrites
// powdertoy.co.uk URLs to /proxy/powdertoy.co.uk/... at runtime.

const ALLOWED = new Set(['powdertoy.co.uk', 'static.powdertoy.co.uk']);

// Headers we never forward in either direction (hop-by-hop + CF internals)
const SKIP_REQ = new Set([
  'host', 'connection', 'transfer-encoding', 'te', 'trailer', 'upgrade',
  'proxy-authorization', 'proxy-authenticate',
  'cf-ray', 'cf-connecting-ip', 'cf-ipcountry', 'cf-visitor',
  'x-forwarded-for', 'x-forwarded-proto', 'x-real-ip',
]);
const SKIP_RESP = new Set([
  'transfer-encoding', 'connection', 'keep-alive', 'te', 'trailer', 'upgrade',
]);

function corsHeaders() {
  return {
    'Access-Control-Allow-Origin': '*',
    'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
    'Access-Control-Allow-Headers': '*',
    'Cross-Origin-Resource-Policy': 'cross-origin',
  };
}

export async function onRequest(context) {
  const { request } = context;
  const url = new URL(request.url);

  // /proxy/<host>/<rest...> → https://<host>/<rest...>?<query>
  const raw = url.pathname.replace(/^\/proxy\//, '');
  const slash = raw.indexOf('/');
  const host = slash === -1 ? raw : raw.slice(0, slash);
  const rest = slash === -1 ? '/' : raw.slice(slash);

  if (!ALLOWED.has(host)) {
    return new Response('Proxy target not allowed', { status: 403 });
  }

  // CORS preflight
  if (request.method === 'OPTIONS') {
    return new Response(null, { status: 204, headers: corsHeaders() });
  }

  const target = `https://${host}${rest}${url.search}`;

  // Forward safe request headers
  const fwdHeaders = new Headers();
  for (const [k, v] of request.headers) {
    if (!SKIP_REQ.has(k.toLowerCase())) fwdHeaders.set(k, v);
  }

  const init = { method: request.method, headers: fwdHeaders, redirect: 'follow' };
  if (request.method !== 'GET' && request.method !== 'HEAD') {
    init.body = request.body;
    init.duplex = 'half';
  }

  try {
    const upstream = await fetch(target, init);

    const respHeaders = new Headers();
    for (const [k, v] of upstream.headers) {
      if (!SKIP_RESP.has(k.toLowerCase())) respHeaders.set(k, v);
    }
    for (const [k, v] of Object.entries(corsHeaders())) {
      respHeaders.set(k, v);
    }

    return new Response(upstream.body, { status: upstream.status, headers: respHeaders });
  } catch (err) {
    return new Response(`Proxy error: ${err.message}`, { status: 502 });
  }
}
