/**
 * Used to acquire meshing updates from the Magic Leap platform.
 */
declare class MLMesher {
    /**
     * This indicates a change to the world mesh state which should be applied to the scene.
     * @param arg- arrays of changes.
     */
    onmesh(arg : MLMeshUpdate[]) : void;
    /**
     * Dispose of this MLMesher instance and stop tracking.
     */
    destroy() : void;
}

declare enum updateType {
    /**
     * Mesh should be added to the scene.
     */
    new = 'new',
    /**
     *  Mesh was previously emitted as new but its data has changed. No action is necessary, but may be desired.
     */
    update = 'update',
    /**
     * Mesh is still valid, but its data has not changed.
     */
    unchanged = 'unchanged',
    /**
     * Mesh is no longer in scope for the meshing system and should be discarded.
     */
    remove = 'remove'
}

/**
 * A single update to the world mesh.
 */
declare class MLMeshUpdate {
    /**
     * A globally unique ID that can be used to identify the world chunk for this mesh.
     * If this ID is the same between updates, then the update represents a change to the existing chunk.
     */
    id : String;
    /**
     * The type of update.  see updateType.
     */
    type : updateType;
    /**
     * The opaque WebGLBuffer for the mesh positions in world space. Represented as three WebGLFloat per vertex.
     * Not in any particular order; indexed by MLMeshUpdate.indexBuffer. Tightly packed stride.
     */
    positionBuffer : WebGLBuffer;
    /**
     * The Float32Array buffer that was uploaded to the positionBuffer.
     */
    positionArray : Float32Array;
    /**
     * The opaque WebGLBuffer for the mesh normals. Represented as three WebGLFloat per vertex.
     * Not in any particular order; indexed by MLMeshUpdate.indexBuffer. Tightly packed stride.
     */
    normalBuffer : WebGLBuffer;
    /**
     * The Float32Array buffer that was uploaded to the normalBuffer.
     */
    normalArray : Float32Array;
    /**
     * The opaque WebGLBuffer for the mesh indices for MLMeshUpdate.position and MLMeshUpdate.normal.
     * Represented as three WebGLShort per triangle. Tightly packed stride.
     */
    indexBuffer : WebGLBuffer;
    /**
     * The Uint16Array buffer that was uploaded to the indexBuffer.
     */
    indexArray : Uint16Array;
    /**
     * The number of indices in MLMeshUpdate.index. Indended to be passed to glDrawElements(GL_TRIANGLES).
     */
    count : number;
}

/**
 * Used to get the current 3D eye tracking position from the Magic Leap platform.
 */
declare class MLEyeTracker {
    /**
     * The current location of the eye cursor, as a world transform. This is probably in front of the
     * camera, in the negative Z.
     */
    fixation : MLTransform;
    /**
     * The individual eye locations and statuses. Note that this does not include the eye fixation (cursor);
     * that is contained in fixation.
     */
    eyes : MLEye;
    /**
     * Dispose of this MLEyeTracker instance and stop tracking.
     */
    destroy() : void;
}

/**
 * Used to receive world planes detected by the Magic Leap platform.
 */
declare class MLPlaneTracker {
    /**
     * This indicates an update to the planes detected by the Magic Leap platform.
     *
     * An update replaces the preview plane state and may contain an entirely different set of planes
     * than the previous update. Plane identity can be tracked via each MLPlaneUpdate's .id property.
     * @param arg- array of MLPlaneUpdate to update.
     */
    onplane(arg : MLPlaneUpdate[]) : void;
    /**
     * Dispose of this MLPlaneTracker instance and stop tracking.
     */
    destroy() : void;
}

/**
 * A single plane detected in the world.
 */
declare class MLPlaneUpdate {
    /**
     * A unique identifier for this plane. If a plane's id is the same between updates,
     * then it is an update to a previous plane.
     */
    id : String;
    /**
     * The world position of the center of this plane.
     */
    position : Float32Array;
    /**
     * The world quaternion of the direction of this plane. Apply this quaternion to the
     * vector (0, 0, 1) to get the plane normal.
     */
    rotation : Float32Array;
    /**
     * The size of the plane in meters.
     *
     * size[0] is the width (x)
     * size[1] is the height (y)
     */
    size : Float32Array;
}

/**
 * Used to acquire hand tracking updates from the Magic Leap platform.
 */
declare class MLHandTracker {
    /**
     * This indicates an update to the user's hand pose detected from the sensors on
     * the Magic Leap platform.
     *
     * Each hand is identified as either 'left' or 'right'. An update replaces the previous
     * hand state; if a hand is not present in any given update that means it has not been
     * detected for the given update loop.
     * @param arg- Array of MLHandUpdate
     */
    onhands(arg: MLHandUpdate[]) : void;
    /**
     * This indicates an update to the detected hand gesture from the sensors on the Magic Leap platform.
     * @param arg- updated hand pose state.
     */
    ongesture(arg : MLGestureUpdate) : void;
    /**
     * Dispose of this MLHandTracker instance and stop tracking.
     */
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

/**
 * A single update to the user's tracked hand pose state.
 */
declare class MLHandUpdate {
    /**
     * The hand detected for this update. Either 'left' or 'right'.
     */
    hand : whichHand;
    /**
     * The pointer transform of the hand pose.
     *
     * This is a ray starting at the tip of the index finger and pointing in the direction
     * of the finger, but it may be based on other pose keypoints as a substitute.
     */
    pointer? : MLTransform;
    /**
     * The grip transform of the hand pose.
     *
     * This is usually a ray starting at the center of the wrist and pointing at the middle finger,
     * but it may be based on other pose keypoints as a substitute.
     */
    grip? : MLTransform;
    /**
     * The rotation of the hand pose detected in world space, as a world quaternion. The rotation is
     * defined as pointing in the direction of the base of the middle finger.
     */
    rotation : Float32Array;
    /**
     * The detected wrist bone position, each as a Float32Array(3) vector vector in world space. The order is:
     * - center
     * - radial
     * - ulnar
     */
    wrist : Float32Array[];
    /**
     * Float32Array[2][5][4][3]
     *
     * The detected hand finger bone positions. The order is right-handed, left-to-right, bottom-to-top:
     *
     * handUpdate.bones[0][0][0] // left (0) thumb (0) base (0) as a Float32Array(3) vector
     * handUpdate.bones[1][0][2] // right (1) thumb (0) tip (2) as a Float32Array(3) vector
     * handUpdate.bones[1][1][3] // right (1) pointer (1) tip (3) as a Float32Array(3) vector
     * handUpdate.bones[0][4][3] // left (0) pinkie (4) tip (3) as a Float32Array(3) vector
     */
    fingers : Float32Array[][][];
    /**
     * The current gesture pose of the hand. See Gestures.
     *
     * A bone may be null, which means it is not currently detected.
     * The thumb has one less bone than the other fingers.
     */
    gesture? : Gestures;
}

/**
 * Represents a single gesture change in the user hand tracking system.
 */
declare class MLGestureUpdate {
    /**
     * The hand detected for this update. Either 'left' or 'right'.
     */
    hand : whichHand;
    /**
     * The detected hand gesture. See Gestures.
     */
    gesture? : Gestures;
    /**
     * A three-component world position vector represting the gesture pointer origin.
     */
    position : Float32Array;
    /**
     * A four-component world quaternion representing the gesture normal.
     *
     * This will be consistent across gesture updates (for tracking grab orientation delta),
     * but do not assume it will point in any particular direction.
     */
    rotation : Float32Array;
}

/**
 * A generic container for position/rotation data in world space.
 */
declare class MLTransform {
    /**
     * A three-component world position vector.
     */
    position : Float32Array;
    /**
     * A four-component world quaternion.
     */
    rotation : Float32Array;
}

/**
 * A single eye state as detected by the platform.
 */
declare class MLEye {
    /**
     * The world position of the eye origin as a vector.
     * Do not use this for checking where the eye is looking; that is what fixation is for.
     */
    position : Float32Array;
    /**
     * The rotation of the eye origin as a world quaternion.
     * Do not use this for checking where the eye is looking; that is what fixation is for.
     */
    rotation : Float32Array;
    /**
     * Whether this eye is currently closed (true) or open (false).
     */
    blink : boolean;
}

/**
 * Used to receive image tracking updates for an image specification from the Magic Leap platform.
 */
declare class MLImageTracker {
    /**
     * @param arg - either MLHandUpdate or null. If non-null, indicates that the image tracking
     * has been updated, and provides the new state. If null, the image has lost tracking.
     */
    ontrack(arg? : MLTrackingUpdate) : void;
}

/**
 * A single update to the tracked image state.
 */
declare class MLTrackingUpdate {
    /**
     * The world position of the tracked image center as a vector.
     */
    position : Float32Array;
    /**
     * The rotation of the tracked image as a world quaternion.
     */
    rotation : Float32Array;
    /**
     * The size of the largest dimension of the image (either X or Y), in meters.
     */
    size: number;
}

interface MagicLeap {
    /**
     * @returns an instance of MLMesher, which can be used to receive world meshing buffer updates from the Magic Leap platform.
     */
    RequestMeshing() : MLMesher;
    /**
     * @returns an instance of MLPlaneTracker, which can be used to receive world planes detected by the Magic Leap platform.
     */
    RequestPlaneTracking() : MLPlaneTracker;
    /**
     * @returns an instance of MLHandTracker, which can be used to receive hand tracking data from the Magic Leap platform.
     */
    RequestHandTracking() : MLHandTracker;
    /**
     * @returns an instance of MLEyeTracker, which can be used to receive eye tracking data from the Magic Leap platform.
     */
    RequestEyeTracking() : MLEyeTracker;
    /**
     * @param image- HTMLImageElement containing the image to track.
     * @param size- The expected size of the widest dimension of the tracker (either X or Y) in meters. This is important for tracking depth.
     * @returns an instance of MLImageTracker, which can be used to receive image tracking updates from the Magic Leap platform.
     */
    RequestImageTracking(image : HTMLImageElement, size : number) : MLImageTracker;
    /**
     * Sets whether the render loop will populate the depth buffer from the meshing system at the start of a frame. If you want natural AR occlusion without messing with mesh data, this is the easiest option.
     *
     * The way this works is that the main framebuffer of your <canvas> (framebuffer 0) will have its depth component pre-rendered at the start of every frame using the unmodified WebVR/WebXR viewport and matrices.
     *
     * Note that you may need to instruct your rendering engine (like THREE.js) to not clear the depth buffer on rendering for this to work as intended.
     *
     * Also note that if you transform the WebXR matrices yourself -- such as changing the projection -- the precomputed depth buffer may no longer align with your render.
     */
    RequestDepthPopulation(populateDepth : Boolean) : void ;
}

interface browser {
    magicLeap : MagicLeap;
}