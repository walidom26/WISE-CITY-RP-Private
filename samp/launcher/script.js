(function () {
  'use strict';
  // ---- الإعدادات: عدّل هنا عنوان سيرفرك ----
  var CONFIG = {
    ip: '127.0.0.1',
    port: 7777,
    discord: 'https://discord.gg/AwZxfNEM5',
    // اختياري: رابط JSON يعيد {"players":12,"max":100} لعرض الحالة الحقيقية. اتركه فارغاً لإخفاء العدّاد.
    statusUrl: ''
  };
  var KEY = 'wc_launcher_v2';
  var $ = function (id) { return document.getElementById(id); };
  var st = { nick: '', ip: CONFIG.ip, port: CONFIG.port, discord: CONFIG.discord };

  function load() { try { var s = JSON.parse(localStorage.getItem(KEY) || '{}'); for (var k in s) if (k in st) st[k] = s[k]; } catch (e) {} }
  function save() { try { localStorage.setItem(KEY, JSON.stringify(st)); } catch (e) {} }

  var RP_NAME = /^[A-Za-z]{2,}_[A-Za-z]{2,}$/;
  function say(t, c) { var m = $('msg'); m.textContent = t; m.className = 'msg ' + (c || ''); }

  function checkNick() {
    var v = $('nick').value.trim(), ok = RP_NAME.test(v) && v.length <= 24;
    $('nick').className = v ? (ok ? 'good' : 'bad') : '';
    $('nickHint').className = 'hint' + (v && !ok ? ' bad' : '');
    return ok;
  }

  function open(title, html) { $('mTitle').textContent = title; $('mBody').innerHTML = html; $('modal').hidden = false; }
  function close() { $('modal').hidden = true; }

  function connect() {
    if (!checkNick()) { say('اكتب الاسم بصيغة Firstname_Lastname', 'bad'); $('nick').focus(); return; }
    var ip = $('ip').value.trim(), port = parseInt($('port').value, 10);
    if (!ip || !(port > 0 && port < 65536)) { say('العنوان أو المنفذ غير صحيح', 'bad'); return; }
    st.nick = $('nick').value.trim(); st.ip = ip; st.port = port; save();
    say('جارٍ فتح اللعبة...', 'ok');
    // روابط الاتصال المباشر (SA-MP / open.mp)
    window.location.href = 'samp://' + ip + ':' + port;
    setTimeout(function () { say('إن لم تفتح اللعبة: ثبّت SA-MP / open.mp ثم أعد المحاولة، واسمك: ' + st.nick); }, 2500);
  }

  function status() {
    if (!CONFIG.statusUrl) { $('players').textContent = '—'; $('status').textContent = 'غير معروفة'; return; }
    fetch(CONFIG.statusUrl, { cache: 'no-store' }).then(function (r) { return r.json(); }).then(function (d) {
      $('players').textContent = d.players + (d.max ? ' / ' + d.max : '');
      $('status').textContent = 'متصل'; $('status').style.color = 'var(--ok)';
    }).catch(function () { $('players').textContent = '—'; $('status').textContent = 'مغلق'; $('status').style.color = 'var(--err)'; });
  }

  // ---- خلفية جسيمات ذهبية خفيفة ----
  function fx() {
    var c = $('fx'), g = c.getContext('2d'), P = [], W, H;
    if (window.matchMedia('(prefers-reduced-motion: reduce)').matches) return;
    function rs() { W = c.width = innerWidth; H = c.height = innerHeight; }
    rs(); addEventListener('resize', rs);
    for (var i = 0; i < 38; i++) P.push({ x: Math.random() * innerWidth, y: Math.random() * innerHeight, r: Math.random() * 1.6 + .4, v: Math.random() * .35 + .1, a: Math.random() * .5 + .2 });
    (function tick() {
      g.clearRect(0, 0, W, H);
      for (var i = 0; i < P.length; i++) {
        var p = P[i]; p.y -= p.v; if (p.y < -5) { p.y = H + 5; p.x = Math.random() * W; }
        g.beginPath(); g.arc(p.x, p.y, p.r, 0, 6.283); g.fillStyle = 'rgba(255,210,74,' + p.a + ')'; g.fill();
      }
      requestAnimationFrame(tick);
    })();
  }

  function init() {
    load(); fx();
    $('nick').value = st.nick; $('ip').value = st.ip; $('port').value = st.port;
    $('nick').addEventListener('input', checkNick);
    $('connect').addEventListener('click', connect);
    $('nick').addEventListener('keydown', function (e) { if (e.key === 'Enter') connect(); });
    $('rulesBtn').addEventListener('click', function () {
      open('قوانين السيرفر', '<ol><li>احترم جميع اللاعبين والإدارة.</li><li>يمنع الغش والهاكات.</li><li>يمنع DM و RK و VDM.</li><li>يمنع Metagaming و Powergaming.</li><li>التزم بقوانين الـ Roleplay.</li><li>يمنع الإعلان عن سيرفرات أخرى.</li><li>أبلغ عن الثغرات ولا تستغلها.</li></ol>');
    });
    $('discordBtn').addEventListener('click', function () { window.open(st.discord, '_blank', 'noopener'); });
    $('settingsBtn').addEventListener('click', function () {
      open('الإعدادات', '<p style="color:var(--mut);line-height:1.9;font-size:14px">عنوان السيرفر والمنفذ يُعدَّلان من الحقول في الشاشة الرئيسية ويُحفظان تلقائياً على جهازك.</p>');
    });
    $('mClose').addEventListener('click', close);
    $('modal').addEventListener('click', function (e) { if (e.target === $('modal')) close(); });
    status(); setInterval(status, 30000);
    setTimeout(function () { $('splash').classList.add('out'); $('app').hidden = false; }, 1400);
  }
  document.addEventListener('DOMContentLoaded', init);
})();
