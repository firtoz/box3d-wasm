import assert from'node:assert/strict';
export const oldLoadingUi='            StartUIFrame((float)sapp_frame_duration());\n            RenderUI(&sc);\n';
export const newLoadingUi='            gpu_loading_ui_begin();\n            StartUIFrame((float)sapp_frame_duration());\n            RenderUI(&sc);\n            gpu_loading_ui_end();\n';
export function isolatedLoadingUi(text:string){assert.equal(text.split(oldLoadingUi).length-1,1);return text.replace(oldLoadingUi,newLoadingUi);}
export function isolatedLoadingGenerator(text:string){const old=oldLoadingUi.replaceAll('\n','\\n'),next=newLoadingUi.replaceAll('\n','\\n');assert.equal(text.split(old).length-1,1);return text.replace(old,next);}
