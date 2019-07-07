var shell = require('shelljs');

shell.rm('-r', '../server/dist/public');

shell.cp('-R', 'dist/', '../server/dist/public/');
