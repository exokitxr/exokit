const {spawn} = require('child_process');
const process = require('os').platform();

    let exokitPath;
        if(process.platform === 'win32'){
        exokitPath = 'scripts\\exokit.cmd';
        }
        else{
        exokitPath = 'scripts/exokit.sh';
        }


    function launch(){
        console.log('launching...')
        const ls = spawn(exokitPath, []);

        ls.stdout.on('data', (data) => {
            console.log(`stdout: ${data}`);
        });
        
        ls.stderr.on('data', (data) => {
            console.log(`stderr: ${data}`);
        });
        
        ls.on('close', (code) => {
            console.log(`child process exited with code ${code}`);
        });
    }
    
    function update(){
        console.log('updating...')
    }