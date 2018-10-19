const xrmpThree = ((
  module = ({
    exports: {},
  })
) => {

const AUDIO_BUFFER_SIZE = 2048;

const _makePlayerMesh = () => {
  const result = new THREE.Object3D();

  const hmd = new THREE.Object3D();
  result.add(hmd);
  result.hmd = hmd;

  const gamepads = [
    new THREE.Object3D(),
    new THREE.Object3D(),
  ];
  for (let i = 0; i < gamepads.length; i++) {
    result.add(gamepads[i]);
  }
  result.gamepads = gamepads;

  return result;
};

class XRMultiplayerTHREE {
  constructor(xrmp) {
    this.xrmp = xrmp;

    this.audioCtx = null;
    this.audioListener = null;

    this.localPlayerMeshes = [];
    this.remotePlayerMeshes = [];
    this.objectMeshes = [];
    this.onopen = null;
    this.onclose = null;
    this.onerror = null;
    this.onsync = null;
    this.onplayerenter = null;
    this.onplayerleave = null;
    this.onobjectadd = null;
    this.onobjectremove = null;
    this.onstateupdate = null;
    this.ongeometry = null;

    xrmp.onopen = e => {
      if (this.onopen) {
        this.onopen(e);
      }
    };
    xrmp.onclose = e => {
      if (this.onclose) {
        this.onclose(e);
      }
    };
    xrmp.onerror = e => {
      if (this.onerror) {
        this.onerror(e);
      }
    };
    xrmp.onsync = e => {
      if (this.onsync) {
        this.onsync(e);
      }
    };
    xrmp.onplayerenter = e => {
      const {player} = e;

      const remotePlayerMesh = _makePlayerMesh();
      remotePlayerMesh.player = player;
      remotePlayerMesh.onupdate = null;

      const positionalAudio = new THREE.PositionalAudio(this.getAudioListener());
      remotePlayerMesh.hmd.add(positionalAudio);
      remotePlayerMesh.positionalAudio = positionalAudio;

      remotePlayerMesh.audioBuffers = [];

      player.onupdate = e => {
        const {matrix: playerMatrix} = e;

        remotePlayerMesh.hmd.position.fromArray(playerMatrix.hmd.position);
        remotePlayerMesh.hmd.quaternion.fromArray(playerMatrix.hmd.quaternion);
        remotePlayerMesh.hmd.updateMatrixWorld();

        for (let j = 0; j < 2; j++) {
          const visible = Boolean(playerMatrix.gamepads[j].enabled[0]);
          remotePlayerMesh.gamepads[j].visible = visible;

          if (visible) {
            remotePlayerMesh.gamepads[j].position.fromArray(playerMatrix.gamepads[j].position);
            remotePlayerMesh.gamepads[j].quaternion.fromArray(playerMatrix.gamepads[j].quaternion);
            remotePlayerMesh.gamepads[j].updateMatrixWorld();
          }
        }

        if (remotePlayerMesh.onupdate) {
          remotePlayerMesh.onupdate();
        }
      };

      this.remotePlayerMeshes.push(remotePlayerMesh);

      this._bindPlayerMeshAudio(remotePlayerMesh);

      if (this.onplayerenter) {
        this.onplayerenter(remotePlayerMesh);
      }
    };
    xrmp.onplayerleave = e => {
      const {player} = e;

      const index = this.remotePlayerMeshes.findIndex(playerMesh => playerMesh.player.id === player.id);
      const remotePlayerMesh = this.remotePlayerMeshes[index];
      this.remotePlayerMeshes.splice(index, 1);

      if (this.onplayerleave) {
        this.onplayerleave(remotePlayerMesh);
      }
    };
    xrmp.onobjectadd = e => {
      const {object} = e;

      const objectMesh = new THREE.Object3D();
      objectMesh.object = object;
      objectMesh.onupdate = null;
      objectMesh.needsUpdate = false;
      this.objectMeshes.push(objectMesh);

      this._bindObjectMesh(objectMesh);

      if (this.onobjectadd) {
        this.onobjectadd(objectMesh);
      }
    };
    xrmp.onobjectremove = e => {
      const {object} = e;

      const index = this.objectMeshes.findIndex(o => o.id === object.id);
      const objectMesh = this.objectMeshes[index];
      this.objectMeshes.splice(index, 1);

      if (this.onobjectremove) {
        this.onobjectremove(objectMesh);
      }
    };
    xrmp.onstateupdate = e => {
      if (this.onstateupdate) {
        this.onstateupdate(e);
      }
    };
    xrmp.ongeometry = e => {
      if (this.ongeometry) {
        this.ongeometry(e);
      }
    };
  }
  createLocalPlayerMesh(id, state) {
    const localPlayerMesh = _makePlayerMesh();

    const localPlayer = this.xrmp.addPlayer(id, state);
    localPlayerMesh.player = localPlayer;

    let mediaStream = null;
    let unsetMediaStream = null;
    localPlayerMesh.getMediaStream = () => mediaStream;
    localPlayerMesh.setMediaStream = newMediaStream => {
      localPlayerMesh.unsetMediaStream();

      const audioCtx = this.getAudioContext();
      const microphoneSourceNode = audioCtx.createMediaStreamSource(newMediaStream);
      const scriptProcessorNode = audioCtx.createScriptProcessor(AUDIO_BUFFER_SIZE, 1, 1);
      scriptProcessorNode.onaudioprocess = e => {
        localPlayer.pushAudio(e.inputBuffer.sampleRate, e.inputBuffer.getChannelData(0));

        e.outputBuffer.getChannelData(0).fill(0);
      };
      microphoneSourceNode.connect(scriptProcessorNode);
      scriptProcessorNode.connect(audioCtx.destination);

      mediaStream = newMediaStream;
      unsetMediaStream = () => {
        scriptProcessorNode.disconnect();

        const tracks = newMediaStream.getTracks();
        for (let i = 0; i < tracks.length; i++) {
          tracks[i].stop();
        }

        mediaStream = null;
      };
    };
    localPlayerMesh.unsetMediaStream = () => {
      if (unsetMediaStream) {
        unsetMediaStream();
        unsetMediaStream = null;
      }
    };

    this.localPlayerMeshes.push(localPlayerMesh);

    return localPlayerMesh;
  }
  createObjectMesh(id, objectMesh = new THREE.Object3D()) {
    const object = this.xrmp.addObject(id);
    objectMesh.object = object;
    objectMesh.onupdate = null;

    this._bindObjectMesh(objectMesh);

    this.objectMeshes.push(objectMesh);

    return objectMesh;
  }
  removeObjectMesh(objectMesh) {
    this.xrmp.removeObject(objectMesh.object);
  }
  getLocalPlayerMeshes() {
    return this.localPlayerMeshes;
  }
  getRemotePlayerMeshes() {
    return this.remotePlayerMeshes;
  }
  getObjectMeshes() {
    return this.objectMeshes;
  }
  getAudioContext() {
    return THREE.AudioContext.getContext();
  }
  getAudioListener() {
    if (!this.audioListener) {
      this.audioListener = new THREE.AudioListener();
    }
    return this.audioListener;
  }
  pushUpdate() {
    for (let i = 0; i < this.localPlayerMeshes.length; i++) {
      const localPlayerMesh = this.localPlayerMeshes[i];
      localPlayerMesh.hmd.position.toArray(localPlayerMesh.player.playerMatrix.hmd.position);
      localPlayerMesh.hmd.quaternion.toArray(localPlayerMesh.player.playerMatrix.hmd.quaternion);

      for (let j = 0; j < 2; j++) {
        const {visible} = localPlayerMesh.gamepads[j];
        localPlayerMesh.player.playerMatrix.gamepads[j].enabled[0] = +visible;

        if (visible) {
          localPlayerMesh.gamepads[j].position.toArray(localPlayerMesh.player.playerMatrix.gamepads[j].position);
          localPlayerMesh.gamepads[j].quaternion.toArray(localPlayerMesh.player.playerMatrix.gamepads[j].quaternion);
        }
      }

      localPlayerMesh.player.pushUpdate();
    }

    for (let i = 0; i < this.objectMeshes.length; i++) {
      const objectMesh = this.objectMeshes[i];

      // console.log('check needs update', objectMesh.needsUpdate);

      if (objectMesh.needsUpdate) {
        objectMesh.position.toArray(objectMesh.object.objectMatrix.position);
        objectMesh.quaternion.toArray(objectMesh.object.objectMatrix.quaternion);

        objectMesh.object.pushUpdate();
        objectMesh.needsUpdate = false;
      }
    }
  }
  close() {
    this.xrmp.close();

    for (let i = 0; i < this.localPlayerMeshes.length; i++) {
      this.localPlayerMeshes[i].unsetMediaStream();
    }
  }
  _bindPlayerMeshAudio(playerMesh) {
    const audioCtx = this.getAudioContext();

    const buffers = [];
    let playing = false;
    const _flushBuffer = () => {
      if (!playing && playerMesh.audioBuffers.length > 0) {
        const [buffer, sampleRate] = playerMesh.audioBuffers.shift();
        const audioBuffer = audioCtx.createBuffer(1, buffer.length, sampleRate);
        audioBuffer.copyToChannel(buffer, 0);

        const bufferSourceNode = audioCtx.createBufferSource();
        bufferSourceNode.connect(audioCtx.destination);
        bufferSourceNode.buffer = audioBuffer;
        bufferSourceNode.start();
        bufferSourceNode.onended = () => {
          bufferSourceNode.onended = null;
          playing = false;

          _flushBuffer();

          bufferSourceNode.disconnect(playerMesh.positionalAudio.getOutput());
        };
        playerMesh.positionalAudio.setNodeSource(bufferSourceNode);
        playing = true;
      }
    };

    playerMesh.player.onaudio = e => {
      playerMesh.audioBuffers.push([e.buffer, e.sampleRate]);

      _flushBuffer();
    };
  }
  _bindObjectMesh(objectMesh) {
    const {object} = objectMesh;

    object.onupdate = e => {
      const {matrix: objectMatrix} = e;

      objectMesh.position.fromArray(objectMatrix.position);
      objectMesh.quaternion.fromArray(objectMatrix.quaternion);
      objectMesh.updateMatrixWorld();

      if (objectMesh.onupdate) {
        objectMesh.onupdate();
      }
    };
  }
}
module.exports.XRMultiplayerTHREE = XRMultiplayerTHREE;

return module.exports;

})(typeof module !== 'undefined' ? module : undefined);

if (typeof window !== 'undefined') {
  window.XRMultiplayerTHREE = xrmpThree.XRMultiplayerTHREE;
}
