export const oldCameraCondition='\tif ( uiCaptured == false || releaseOrUnfocus )';
export const newCameraCondition='\tif ( uiCaptured == false || releaseOrUnfocus || e->type == SAPP_EVENTTYPE_KEY_UP )';
export const captureCall='uiCaptured = gpu_ui_capture_event( e, HandleEvent( e ) );';
export const capturedRelease=captureCall+'\n\t\t// Captured presses stay in the UI; releases must clear prior host input.\n\t\tif ( uiCaptured && e->type == SAPP_EVENTTYPE_KEY_UP )\n\t\t{\n\t\t\tSetKeyDown( e->key_code, false );\n\t\t}';
export function repairMain(text:string){for(const x of [captureCall,oldCameraCondition])if(text.split(x).length!==2)throw Error('UI10 source drift '+x);return text.replace(captureCall,capturedRelease).replace(oldCameraCondition,newCameraCondition);}
