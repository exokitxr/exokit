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
    console.log('launching...')
    let flagsString = '-';
    if(flags.length > 1){
        flags.forEach(flag => {
            flagsString += flag;
        });
    }
    else{
        flagsString += flag[0];
    }
    spawn(exokitPath, [flagsString]);
}

function update(){
    console.log('updating...')
}