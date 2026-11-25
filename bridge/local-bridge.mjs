#!/usr/bin/env node
/** Language-agnostic bridge: child stdout JSONL -> authenticated WSS relay. */
import { spawn } from 'node:child_process';
import { createInterface } from 'node:readline';
const divider = process.argv.indexOf('--');
const cmd = divider>=0?process.argv.slice(divider+1):[];
if(!cmd.length){console.error('Usage: node bridge/local-bridge.mjs -- <program> [args...]');process.exit(2);}
const participantId=process.env.GALAXY_PARTICIPANT_ID || 'student1';
const url=process.env.GALAXY_WS || 'ws://127.0.0.1:8787/ws';
const token=process.env.GALAXY_TOKEN || (url.startsWith('ws://127.0.0.1:')?'local-demo-token':'');
if(!token){console.error('Set GALAXY_TOKEN for non-local servers');process.exit(2);}
const ws=new WebSocket(url);
let child,ready=false,closed=false;
function shutdown(){if(closed)return;closed=true;if(child&&!child.killed)child.kill();if(ws.readyState===WebSocket.OPEN)ws.close();}
const timeout=setTimeout(()=>{if(!ready){console.error('[bridge] connection timeout');shutdown();}},10000);
ws.addEventListener('open',()=>ws.send(JSON.stringify({type:'hello',role:'producer',participantId,token})));
ws.addEventListener('message',(event)=>{
  let msg;try{msg=JSON.parse(event.data)}catch{return;}
  if(msg.type==='hello_ack'&&msg.role==='producer'&&!ready){
    ready=true;clearTimeout(timeout);
    console.error(`[bridge] connected: ${participantId}`);
    child=spawn(cmd[0],cmd.slice(1),{stdio:['pipe','pipe','inherit'],shell:false});
    child.stdin.on('error', e => {
      if (e.code !== 'EPIPE' && e.code !== 'ERR_STREAM_DESTROYED') console.error('[bridge] stdin error:', e.message);
      shutdown();
    });
    child.on('error',e=>{console.error('[bridge] launch error:',e.message);shutdown();});
    child.on('exit',(code)=>{console.error('[bridge] program exited:',code);shutdown();});
    child.stdin.write(JSON.stringify({type:'init',participantId})+'\n');
    const lines=createInterface({input:child.stdout,crlfDelay:Infinity});
    lines.on('line',line=>{
      if(!line.trim())return;
      if(Buffer.byteLength(line,'utf8')>8000){console.error('[bridge] line too long');return;}
      try{
        const message=JSON.parse(line);
        if(!['acquire','release','reset'].includes(message.type))throw new Error('Unsupported message type');
        if(ws.readyState===WebSocket.OPEN)ws.send(JSON.stringify(message));
      }catch(e){console.error('[bridge] invalid stdout JSONL:',e.message);}
    });return;
  }
  if(child&&child.stdin.writable&&['ack','error'].includes(msg.type))child.stdin.write(JSON.stringify(msg)+'\n');
  if(msg.type==='error')console.error('[relay]',msg.message);
});
ws.addEventListener('close',()=>{clearTimeout(timeout);console.error('[bridge] relay disconnected');shutdown();});
ws.addEventListener('error',()=>console.error('[bridge] websocket connection error'));
process.on('SIGINT',()=>{shutdown();process.exit(130)});
process.on('SIGTERM',()=>{shutdown();process.exit(143)});
