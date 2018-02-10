# exokit-macos

install the prerequisites:
```
brew install glfw
yarn
```

install and run [zeo](https://github.com/modulesio/zeo), the game server:
```
cd ..
git clone https://github.com/modulesio/zeo
cd zeo
yarn
./index.js
```

once zeo is running, you'll see an IP address:
```
Local URL: http://127.0.0.1:8000
Remote URL: http://8.9.10.11:8000
zeo>
```

visit http://127.0.0.1:8000 and verify the game world loads.
If all went well, close the browser and run exokit:

```
node . http://127.0.0.1:8000
```

