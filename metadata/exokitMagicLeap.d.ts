
declare class MLMesher {
    onmesh(arg : MLMeshUpdate[]) : void;
    destroy() : void;
}

declare enum updateType {
    new = 'new',
    update = 'update',
    unchanged = 'unchanged',
    remove = 'remove'
}

declare class MLMeshUpdate {
    id : String;
    type : updateType;
    positionBuffer : WebGLBuffer;
    positionArray : Float32Array;
    normalBuffer : WebGLBuffer;
    normalArray : Float32Array;
    indexBuffer : WebGLBuffer;
    indexArray : Uint16Array;
    count : number;  // docs had as 'Number'.  Is this an object or a literal?
}

declare class MLEyeTracker {
    fixation : MLTransform;
    eyes : MLEye;
    destroy() : void;
}

declare class MLPlaneTracker {
    onplane(arg : MLPlaneUpdate[]) : void;
    destroy() : void;
}

declare class MLPlaneUpdate {
    id : String;
    position : Float32Array;
    rotation : Float32Array;
    size : Float32Array;
}

declare class MLHandTracker {
    onhands(arg: MLHandUpdate[]) : void;
    ongesture(arg : MLGestureUpdate) : void;
    destroy() : void;
}

declare enum whichHand {
    left = 'left',
    right = 'right'
}

declare enum Gestures {
    finger = 'finger',
    fist = 'fist',
    pinch = 'pinch',
    thumb = 'thumb',
    l = 'l',
    openHandBack = 'openHandBack',
    ok = 'ok',
    c = 'c'
}

declare class MLHandUpdate {
    hand : whichHand;
    pointer : MLTransform; // what does the '?' mean?
    grip : MLTransform; // what does the '?' mean?
    rotation : Float32Array;
    wrist : Float32Array[];
    fingers : Float32Array[][][];
    gesture? : Gestures;
}

declare class MLGestureUpdate {
    hand : whichHand;
    gesture? : Gestures;
    position : Float32Array;
    rotation : Float32Array;    
}

declare class MLTransform {
    position : Float32Array;
    rotation : Float32Array;        
}

/*
 * This is compented out until the duplicate class name can be resolved.

declare class MLEyeTracker {
    oneye(arg: MLEye) : void;
}
 */

declare class MLEye {
    position : Float32Array;
    rotation : Float32Array;
    blink : boolean;    
}

declare class MLImageTracker {
    ontrack(arg? : MLTrackingUpdate) : void;
}

declare class MLTrackingUpdate {
    position : Float32Array;
    rotation : Float32Array;        
    size: number;    
}

interface MagicLeap {
    RequestMeshing() : MLMesher;
    RequestPlaneTracking() : MLPlaneTracker;
    RequestHandTracking() : MLHandTracker;
    RequestEyeTracking() : MLEyeTracker;
    RequestImageTracking(image : HTMLImageElement , size : number) : MLImageTracker;
    RequestDepthPopulation(populateDepth : Boolean);
}

interface browser {
    magicLeap : MagicLeap;
}