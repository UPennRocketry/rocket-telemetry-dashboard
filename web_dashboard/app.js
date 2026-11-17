/* ── flight model: Pathfinder II, Flight 04 ────────────────────── */
const BURN=1.8, APO=14.6, MAIN=42.1, LAND=127.3;
const APO_ALT=3386, BURN_ALT=620, MAIN_ALT=1874, DOWNRANGE=2220; // ft (0.42 mi)
const TOP_MPH=284, MAX_G=12.2;

function alt(t){
  if(t<=0) return 0;
  if(t<BURN) return BURN_ALT*Math.pow(t/BURN,1.85);
  if(t<APO)  return APO_ALT-(APO_ALT-BURN_ALT)*Math.pow((APO-t)/(APO-BURN),2);
  if(t<MAIN) return APO_ALT-55*(t-APO);
  if(t<LAND) return MAIN_ALT-22*(t-MAIN);
  return 0;
}
function down(t){
  if(t<=0) return 0;
  if(t<BURN) return 60*Math.pow(t/BURN,2);
  if(t<APO)  return 60+460*Math.pow((t-BURN)/(APO-BURN),1.3);
  if(t<MAIN) return 520+700*((t-APO)/(MAIN-APO));
  if(t<LAND) return 1220+1000*((t-MAIN)/(LAND-MAIN));
  return DOWNRANGE;
}
function vel(t){
  if(t<=0) return 0;
  if(t<BURN) return TOP_MPH*(t/BURN);
  if(t<APO)  return TOP_MPH*((APO-t)/(APO-BURN));
  if(t<MAIN) return 37;
  if(t<LAND) return 15;
  return 0;
}
function gee(t){
  if(t<=0) return 1.0;
  if(t<BURN) return 1+(MAX_G-1)*Math.sin(Math.PI*Math.min(t/BURN,1));
  if(t<APO)  return 0.9*((APO-t)/(APO-BURN));
  return 1.0;
}
function phaseOf(t){
  if(t<0) return "pad";
  if(t<BURN) return "boost";
  if(t<APO) return "coast";
  if(t<LAND) return "descent";
  return "summary";
}
const clockStr=t=>{
  const tenths=Math.round(Math.abs(t)*10),m=Math.floor(tenths/600),s=(tenths%600)/10;
  return (t<0?"T−":"T+")+"00:"+String(m).padStart(2,"0")+":"+s.toFixed(1).padStart(4,"0");
};
const ft=n=>Math.round(n).toLocaleString("en-US");
const mi=n=>(n/5280).toFixed(2);
const esc=s=>String(s).replace(/&/g,"&amp;").replace(/</g,"&lt;");

/* ── plot geometry ─────────────────────────────────────────────── */
const L=78,R=968,GY=430,TY=56,ALTMAX=3600,XMAX=2400;
const px=x=>L+(R-L)*(x/XMAX), py=a=>GY-(GY-TY)*(a/ALTMAX);

const MARKS=[
  {t:BURN, name:"Burnout", v:ft(BURN_ALT)+" ft · T+00:01.8", dx:-14, dy:-26, anchor:"end"},
  {t:APO,  name:"Apogee",  v:ft(APO_ALT)+" ft · T+00:14.6",  dx:14,  dy:-26, anchor:"start"},
  {t:MAIN, name:"Main",    v:ft(MAIN_ALT)+" ft · T+00:42.1", dx:14,  dy:-10, anchor:"start"},
  {t:LAND, name:"Landed",  v:mi(DOWNRANGE)+" mi · T+02:07.3",dx:-10, dy:-56, anchor:"end"}
];

function path(to){
  let d="",n=240;
  for(let i=0;i<=n;i++){const t=to*i/n;d+=(i?"L":"M")+px(down(t)).toFixed(1)+" "+py(alt(t)).toFixed(1);}
  return d;
}
/* a white plate keeps labels legible over both the deep sky and the pale horizon */
function plate(x,y,name,val,anchor){
  const w=Math.max(name.length*7.3,val.length*6.5)+18, h=36;
  const rx=anchor==="end"?x-w:x;
  const tx=anchor==="end"?x-9:x+9;
  return `<g><rect x="${rx.toFixed(1)}" y="${(y-24).toFixed(1)}" width="${w.toFixed(1)}" height="${h}" rx="4" fill="var(--plate)" stroke="var(--rule)" stroke-width=".75"/>`+
    `<text class="mk-name" x="${tx.toFixed(1)}" y="${(y-8).toFixed(1)}" text-anchor="${anchor}">${esc(name)}</text>`+
    `<text class="mk-val" x="${tx.toFixed(1)}" y="${(y+6).toFixed(1)}" text-anchor="${anchor}">${esc(val)}</text></g>`;
}

function drawPlot(t,ph){
  const live=ph!=="summary"&&ph!=="pad";
  let s=`<defs>
    <linearGradient id="sky" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" style="stop-color:var(--sky-1)"/><stop offset="0.28" style="stop-color:var(--sky-2)"/>
      <stop offset="0.58" style="stop-color:var(--sky-3)"/><stop offset="0.82" style="stop-color:var(--sky-4)"/>
      <stop offset="1" style="stop-color:var(--sky-5)"/>
    </linearGradient>
    <linearGradient id="plume" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" style="stop-color:var(--hot-2)" stop-opacity=".95"/>
      <stop offset="1" style="stop-color:var(--hot)" stop-opacity="0"/>
    </linearGradient>
  </defs>`;
  s+=`<rect x="0" y="0" width="1000" height="${GY}" fill="url(#sky)"/>`;
  s+=`<rect x="0" y="${GY}" width="1000" height="${520-GY}" fill="var(--ground)"/>`;
  // altitude grid
  for(let a=1000;a<=3000;a+=1000){
    s+=`<line x1="0" y1="${py(a)}" x2="1000" y2="${py(a)}" stroke="var(--grid)" stroke-width="1" stroke-dasharray="2 7"/>`;
    s+=`<text class="tick" x="14" y="${py(a)-7}" fill="var(--grid)">${ft(a)} ft</text>`;
  }
  // ground + downrange ticks
  s+=`<line x1="0" y1="${GY}" x2="1000" y2="${GY}" stroke="var(--ink)" stroke-width="1.5"/>`;
  [[0,"pad"],[1000,"1,000 ft"],[2000,"2,000 ft"]].forEach(([x,lab])=>{
    s+=`<line x1="${px(x)}" y1="${GY}" x2="${px(x)}" y2="${GY+7}" stroke="var(--dim-2)" stroke-width="1"/>`;
    s+=`<text class="tick" x="${px(x)}" y="${GY+22}" text-anchor="middle" fill="var(--dim)">${lab}</text>`;
  });
  // pad
  s+=`<ellipse cx="${px(0)}" cy="${GY}" rx="7" ry="9" fill="none" stroke="var(--ink)" stroke-width="1.6"/>`;
  if(ph==="pad") s+=`<path d="M${px(0)} ${GY-13} l5 11 h-10 z" fill="var(--hot)"/>`;
  // predicted profile the flown line is tracking against
  if(live&&!radioMode) s+=`<path d="${path(LAND)}" fill="none" stroke="var(--grid)" stroke-width="1.5" stroke-dasharray="3 7"/>`;
  // flown path, with a soft plume glow behind it
  if((radioMode && radioPoints.length) || (!radioMode && t>0)){
    const d=radioMode ? radioPoints.map((p,i)=>(i?"L":"M")+px(p.distance).toFixed(1)+" "+py(p.altitude).toFixed(1)).join("") : path(Math.min(t,LAND));
    s+=`<path d="${d}" fill="none" stroke="var(--hot-2)" stroke-width="7" stroke-opacity=".28" stroke-linecap="round"/>`;
    s+=`<path d="${d}" fill="none" stroke="var(--hot)" stroke-width="2.6" stroke-linecap="round" stroke-linejoin="round"/>`;
  }
  // markers dropped in the rocket's wake
  if(!radioMode) MARKS.forEach(m=>{
    if(t<m.t-0.01) return;
    const x=px(down(m.t)), y=py(alt(m.t));
    const lx=x+m.dx, ly=y+m.dy;
    s+=`<line x1="${x}" y1="${y}" x2="${lx}" y2="${ly+2}" stroke="var(--plate)" stroke-width="1.25"/>`;
    s+=plate(lx,ly,m.name,m.v,m.anchor);
    s+=`<circle cx="${x}" cy="${y}" r="4.5" fill="var(--panel)" stroke="var(--ink)" stroke-width="2"/>`;
  });
  // the rocket
  if((radioMode&&radioPoints.length) || (!radioMode&&t>0&&t<LAND)){
    const sample=radioMode?radioPoints[radioPoints.length-1]:null;
    const altitude=sample?sample.altitude:alt(t), distance=sample?sample.distance:down(t);
    const x=px(distance), y=py(altitude);
    s+=`<circle class="pulse" cx="${x}" cy="${y}" r="14" fill="var(--hot-2)"/>`;
    s+=`<circle cx="${x}" cy="${y}" r="6" fill="var(--hot)" stroke="var(--panel)" stroke-width="1.6"/>`;
    s+=plate(x+16,y+(t<APO?34:40),ft(altitude)+" ft",ft(distance)+" ft from pad","start");
  }
  document.getElementById("svg").innerHTML=s;
}

/* ── rails ─────────────────────────────────────────────────────── */
const PORTS=["/dev/tty.usbserial-A50285BI — 915 MHz","/dev/tty.usbmodem14201"];
function fields(t,ph){
  if(ph==="pad"){
    return `<div class="check"><b>PAD CHECKLIST</b>
      <label><input type="checkbox" id="c1" checked><span>GPS lock — 9 satellites</span></label>
      <label><input type="checkbox" id="c2" checked><span>Range clear</span></label>
      <label><input type="checkbox" id="c3"><span>Igniter continuity</span></label>
      <label><input type="checkbox" id="c4"><span>Altimeter armed</span></label></div>
      <div class="field"><b>ALTITUDE, SEA LEVEL</b><i>4,431 ft</i></div>
      <div class="field"><b>WIND</b><i>4 mph SSW</i></div>`;
  }
  if(ph==="summary"){
    return `<div class="field"><b>TOP SPEED</b><i>${TOP_MPH} mph</i></div>
      <div class="field"><b>MAX ACCELERATION</b><i>${MAX_G} G</i></div>
      <div class="field"><b>APOGEE</b><i>${ft(APO_ALT)} ft</i></div>
      <div class="field"><b>RECOVERED</b><i>${mi(DOWNRANGE)} mi out</i></div>`;
  }
  const up=t<APO;
  return `<div class="field live"><b>ALTITUDE</b><i>${ft(alt(t))} ft</i></div>
    <div class="field live"><b>${up?"CLIMB":"DESCENT"} RATE</b><i><span class="arrow">${up?"▲":"▼"}</span>${Math.round(vel(t))} mph</i></div>
    <div class="field"><b>ACCELERATION</b><i>${gee(t).toFixed(1)} G</i></div>
    <div class="field"><b>DISTANCE FROM PAD</b><i>${ft(down(t))} ft</i></div>`;
}
function bars(n){
  let s=`<svg width="20" height="13" viewBox="0 0 20 13" aria-hidden="true">`;
  for(let i=0;i<4;i++) s+=`<rect x="${i*5}" y="${11-(i+1)*2.7}" width="3.2" height="${(i+1)*2.7}" rx="1" fill="${i<n?"var(--go)":"var(--rule)"}"/>`;
  return s+`</svg>`;
}
function linkCluster(ph){
  return `<div><b>LINK</b><i>${bars(ph==="descent"?3:4)} 915 MHz</i></div>
    <div><b>AIRFRAME</b><i>${ph==="pad"?"8.1 V":"7.9 V"}</i></div>
    <div><b>SATS</b><i>9</i></div>
    <div><b>TEMP</b><i>72°F</i></div>`;
}
/* the next event, beside the mission clock — every time on the screen in one block */
function countdown(t,ph){
  if(ph==="boost")   return `<i>${Math.max(0,BURN-t).toFixed(1)} s</i><small>BURNOUT IN</small>`;
  if(ph==="coast")   return `<i>${Math.max(0,APO-t).toFixed(1)} s</i><small>APOGEE IN</small>`;
  if(ph==="descent") return `<i>${Math.max(0,LAND-t).toFixed(0)} s</i><small>DOWN IN</small>`;
  return "";
}
function bottom(t,ph){
  if(ph==="pad") return `<div class="pred"><b>SERIAL PORT</b></div>
    <select id="port" aria-label="Serial port">${PORTS.map(p=>`<option>${esc(p)}</option>`).join("")}</select>
    <select id="replay" aria-label="Replay a saved flight"><option>Replay a flight…</option><option>Flight 03 · 12 Oct</option><option>Flight 02 · 28 Sep</option></select>
    <button class="pill go spacer" id="arm" type="button">Arm for launch</button>`;
  if(ph==="boost"||ph==="coast") return `<div class="pred"><b>PROJECTED APOGEE</b><i>${ft(APO_ALT)} ft</i></div>`;
  if(ph==="descent") return `<div class="pred"><b>CANOPY</b><i>${t<MAIN?"Drogue":"Main"}</i></div>
    <div class="pred"><b>GROUND SPEED</b><i>${t<MAIN?25:16} mph downwind</i></div>`;
  return `<div class="flights"><b>EARLIER FLIGHTS</b>
      <a href="#f03">Flight 03 · 3,102 ft</a><a href="#f02">Flight 02 · 2,874 ft</a><a href="#f01">Flight 01 · 1,640 ft</a></div>
    <button class="pill spacer" id="replay-flight" type="button">Replay flight</button>
    <button class="pill" id="export" type="button">Export CSV</button>`;
}

const PH=[["pad","PAD"],["boost","BOOST"],["coast","COAST"],["descent","DESCENT"],["summary","SUMMARY"]];
const PH_T={pad:-12,boost:1.2,coast:8,descent:56,summary:LAND};
document.getElementById("phases").innerHTML=PH.map(([k,l])=>`<button type="button" data-ph="${k}" aria-pressed="false">${l}</button>`).join("");

let T=-12, playing=false, raf=null, last=0;
let radioMode=false, radioPoints=[], radioLastSequence=null, radioReceivedAt=0;
const checklist={c1:true,c2:true,c3:false,c4:false};
let selectedPort=PORTS[0];
function render(){
  const ph=phaseOf(T);
  if(radioMode){
    const sample=radioPoints[radioPoints.length-1];
    document.getElementById("clock").innerHTML=sample
      ? `${clockStr(sample.t)}<small>SIMULATED MISSION CLOCK — RADIO</small>`
      : `--<small>WAITING FOR SIMULATED RADIO PACKET</small>`;
    document.getElementById("cdn").innerHTML="";
    document.getElementById("fields").innerHTML=sample
      ? `<div class="field live"><b>ALTITUDE</b><i>${ft(sample.altitude)} ft</i></div>
         <div class="field live"><b>SPEED</b><i>${sample.velocity.toFixed(1)} mph</i></div>
         <div class="field"><b>ACCELERATION</b><i>${sample.acceleration.toFixed(2)} G</i></div>
         <div class="field"><b>DISTANCE FROM PAD</b><i>${ft(sample.distance)} ft</i></div>`
      : `<div class="field"><b>SIMULATED FLIGHT</b><i>Waiting for data</i></div>`;
    document.getElementById("link").innerHTML=`<div><b>LINK</b><i>433 MHz · SIMULATED</i></div>`;
    document.getElementById("bot").innerHTML=`<div class="pred"><b>RADIO MODE</b><i>Plot shows received samples only</i></div>`;
    drawPlot(T,ph);
    document.querySelectorAll("#phases button").forEach(b=>b.setAttribute("aria-pressed",sample && b.dataset.ph===ph ? "true" : "false"));
    document.getElementById("t").value=T;
    return;
  }
  document.getElementById("clock").innerHTML=
    (ph==="summary"?`02:07.3<small>FLIGHT DURATION — FROZEN AT TOUCHDOWN</small>`
     :ph==="pad"?`${clockStr(T)}<small>HOLDING — CLOCK STARTS AT LIFTOFF</small>`
     :`${clockStr(T)}<small>MISSION CLOCK</small>`);
  document.getElementById("cdn").innerHTML=countdown(T,ph);
  document.getElementById("fields").innerHTML=fields(T,ph);
  if(ph==="pad"){
    Object.entries(checklist).forEach(([id,checked])=>{document.getElementById(id).checked=checked;});
  }
  document.getElementById("link").innerHTML=linkCluster(ph);
  document.getElementById("bot").innerHTML=bottom(T,ph);
  if(ph==="pad") document.getElementById("port").value=selectedPort;
  drawPlot(T,ph);
  document.querySelectorAll("#phases button").forEach(b=>b.setAttribute("aria-pressed",b.dataset.ph===ph));
  const sl=document.getElementById("t"); if(document.activeElement!==sl) sl.value=T;
  sl.style.setProperty("--progress",`${(Number(sl.value)-Number(sl.min))/(Number(sl.max)-Number(sl.min))*100}%`);
}
function step(ts){
  if(!playing) return;
  const dt=last?(ts-last)/1000:0; last=ts;
  T=Math.min(LAND+4.7,T+dt*7);
  if(T>=LAND+4.6){T=LAND+4.7;stop();}
  render(); if(playing) raf=requestAnimationFrame(step);
}
function stop(){playing=false;last=0;document.getElementById("play").textContent="Fly it";if(raf)cancelAnimationFrame(raf);}
function play(){
  if(radioMode) return;
  if(playing){stop();render();return;}
  playing=true;T=-12;last=0;document.getElementById("play").textContent="Stop";raf=requestAnimationFrame(step);
}
document.getElementById("play").addEventListener("click",play);
document.getElementById("t").addEventListener("input",e=>{if(radioMode)return;stop();T=parseFloat(e.target.value);render();});
document.getElementById("phases").addEventListener("click",e=>{
  if(radioMode) return;
  const b=e.target.closest("button"); if(!b) return;
  stop(); T=PH_T[b.dataset.ph]; render();
});
document.getElementById("fields").addEventListener("change",e=>{
  if(e.target.matches('input[type="checkbox"]') && Object.hasOwn(checklist,e.target.id)){
    checklist[e.target.id]=e.target.checked;
  }
});
document.getElementById("bot").addEventListener("change",e=>{
  if(e.target.id==="port") selectedPort=e.target.value;
  if(e.target.id==="replay" && e.target.selectedIndex>0){
    alert("Saved flight replay is not connected in this mockup.");
    e.target.selectedIndex=0;
  }
});
document.getElementById("bot").addEventListener("click",e=>{
  if(e.target.id==="arm"){
    alert("This is a simulation. No vehicle is connected and no launch command was sent.");
  }else if(e.target.id==="replay-flight"){
    play();
  }else if(e.target.id==="export"){
    const rows=["elapsed_seconds,phase,altitude_ft,downrange_ft,velocity_mph,acceleration_g"];
    for(let tick=0;tick<=Math.round(LAND*10);tick++){
      const t=tick/10;
      rows.push([t.toFixed(1),phaseOf(t),alt(t).toFixed(1),down(t).toFixed(1),vel(t).toFixed(1),gee(t).toFixed(2)].join(","));
    }
    const url=URL.createObjectURL(new Blob([rows.join("\n")+"\n"],{type:"text/csv;charset=utf-8"}));
    const link=document.createElement("a");
    link.href=url;link.download="pathfinder-ii-flight-04-simulation.csv";
    document.body.append(link);link.click();link.remove();
    setTimeout(()=>URL.revokeObjectURL(url),1000);
  }else if(e.target.closest(".flights a")){
    e.preventDefault();
    alert("Earlier flights are not connected in this mockup.");
  }
});

const radioStatus=document.getElementById("radio-status");
const connectRadio=document.getElementById("connect-radio");
const disconnectRadio=document.getElementById("disconnect-radio");
let serialReader=null, serialTask=null, radioError="", serialCloseError="";

function setRadioControls(active){
  connectRadio.hidden=active;
  disconnectRadio.hidden=!active;
  document.getElementById("play").disabled=active;
  document.getElementById("t").disabled=active;
  document.querySelectorAll("#phases button").forEach(button=>button.disabled=active);
}
function updateRadioStatus(){
  if(!radioMode) return;
  if(radioError) radioStatus.textContent=radioError;
  else if(!radioReceivedAt) radioStatus.textContent="Receiver connected — waiting for SIM_FLIGHT packets.";
  else if(Date.now()-radioReceivedAt>5000) radioStatus.textContent="SIMULATED RADIO — no packet for over 5 seconds; displaying last received data.";
  else radioStatus.textContent=`SIMULATED RADIO · 433 MHz · packet ${radioLastSequence}`;
}
function acceptRadioLine(line){
  if(!line.startsWith("SIM_FLIGHT,")) return;
  const sample=FlightTelemetry.parseFlightPacket(line);
  if(!sample){
    radioError="Invalid SIM_FLIGHT packet ignored; check firmware and serial data.";
    updateRadioStatus();
    return;
  }
  if(radioLastSequence!==null && sample.seq<=radioLastSequence){
    if(sample.seq===radioLastSequence ||
       (sample.t>=radioPoints[radioPoints.length-1].t && Date.now()-radioReceivedAt<=5000)) return;
    radioPoints=[];
  }
  if(radioPoints.length && sample.t<radioPoints[radioPoints.length-1].t) radioPoints=[];
  radioLastSequence=sample.seq;
  radioPoints.push(sample);
  if(radioPoints.length>145) radioPoints.shift();
  radioReceivedAt=Date.now();
  radioError="";
  T=sample.t;
  updateRadioStatus();
  render();
}
async function readRadio(port){
  const decoder=new TextDecoder();
  const lines=new FlightTelemetry.LineBuffer(acceptRadioLine,()=>{
    radioError="Serial line too long; discarded.";
    updateRadioStatus();
  });
  try{
    serialReader=port.readable.getReader();
    while(true){
      const {value,done}=await serialReader.read();
      if(done) break;
      lines.push(decoder.decode(value,{stream:true}));
    }
    if(radioMode) radioError="Receiver serial stream closed. Disconnect and reconnect to try again.";
  }catch(error){
    if(radioMode) radioError=`Receiver serial error: ${error.message}`;
  }finally{
    if(serialReader){serialReader.releaseLock();serialReader=null;}
    try { await port.close(); }
    catch(error) {
      serialCloseError=`Could not close receiver port: ${error.message}`;
      if(radioMode) radioError=serialCloseError;
    }
    updateRadioStatus();
  }
}
connectRadio.addEventListener("click",async()=>{
  if(!("serial" in navigator)){
    radioStatus.textContent="Web Serial unavailable. Use Chrome or Edge on localhost.";
    return;
  }
  connectRadio.disabled=true;
  let port=null, opened=false;
  try{
    port=await navigator.serial.requestPort();
    await port.open({baudRate:115200});
    opened=true;
    await port.setSignals({dataTerminalReady:true});
    stop();
    radioMode=true; radioPoints=[]; radioLastSequence=null; radioReceivedAt=0; radioError=""; serialCloseError=""; T=-12;
    setRadioControls(true);
    updateRadioStatus();
    render();
    serialTask=readRadio(port);
  }catch(error){
    radioStatus.textContent=`Could not connect receiver: ${error.message}`;
    if(opened){
      try { await port.close(); }
      catch(closeError) { radioStatus.textContent+=` Could not close port: ${closeError.message}`; }
    }
  }finally{
    connectRadio.disabled=false;
  }
});
disconnectRadio.addEventListener("click",async()=>{
  disconnectRadio.disabled=true;
  radioMode=false;
  try{
    if(serialReader) await serialReader.cancel();
    if(serialTask) await serialTask;
    if(serialCloseError) throw new Error(serialCloseError);
  }catch(error){
    radioStatus.textContent=`Error disconnecting receiver: ${error.message}`;
    disconnectRadio.disabled=false;
    radioMode=true;
    return;
  }
  serialTask=null;
  setRadioControls(false);
  disconnectRadio.disabled=false;
  radioStatus.textContent="Demo mode — simulated flight, no radio connected.";
  T=-12;
  render();
});
setInterval(updateRadioStatus,1000);
render();
