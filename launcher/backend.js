const {spawn} = require('child_process');
const process = require('os').platform();

    let exokitPath;
        if(process.platform === 'win32'){
        exokitPath = 'scripts\\exokit.cmd';
        }
        else{
        exokitPath = 'scripts/exokit.sh';
        }


    function launch(flags){
        let flagsString = '-';
        if(flags.length > 1){
            flags.forEach(flag => {
                flagsString += flag;
            });
        }
        else{
            flagsString += flag[0];
        }
        console.log('backend flags: ', flagsString)
        console.log('launching...')
        const ls = spawn(exokitPath, [flagsString]);
        
    //         // arg2 is url, arg3 is flags, they can be empty or filled.
    // if(arg3.length > 0 && arg2.length > 0){ // launch with both flag and url
    //     spawn(exokitPath, [arg2, '-' + arg3], {detached: true, stdio: ['ignore', 'ignore', 'ignore']});
    //   }
    //   if(arg3.length > 0){ // launch with just a flag
    //     spawn(exokitPath, ['-' + arg3], {detached: true, stdio: ['ignore', 'ignore', 'ignore']});
    //   }
    //   if(arg2.length > 0){ // launch with just a URL parameter
    //     spawn(exokitPath, [arg2], {detached: true, stdio: ['ignore', 'ignore', 'ignore']});
    //   }
    //   else{ // launch with nothing extra
    //     if(process.platform === 'darwin'){
    //       spawn('sh', [exokitPath], {shell: true, detached: true});
    //     }
    //     else{
    //       spawn(exokitPath, {detached: true, stdio: ['ignore', 'ignore', 'ignore']});
    //     }
    //   }
    }

    function update(){
        console.log('updating...')
    }