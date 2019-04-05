#!/usr/bin/env node

const path = require('path');
const fs = require('fs');
const find = require('find');
const murmurhash3 = require('murmurhash3');

let dirname = process.argv[2];

let assetStats = [];
const FILE = 1;
const DIRECTORY = 2;

dirname = path.join(dirname, 'android', 'app', 'assets');
new Promise((accept, reject) => {
  find.file(dirname, files => {
    Promise.all(files.map(file => new Promise((accept, reject) => {
      fs.lstat(file, (err, stat) => {
        if (!err) {
          file = file.slice(dirname.length + 1);
          const parentFile = path.dirname(file);

          Promise.all([
            new Promise((accept, reject) => {
              murmurhash3.murmur32(file, (err, hashValue) => {
                if (!err) {
                  // console.log('got hash', file, hashValue.toString(16));
                  accept(hashValue);
                } else {
                  reject(err);
                }
              });
            }),
            new Promise((accept, reject) => {
              murmurhash3.murmur32(parentFile, (err, hashValue) => {
                if (!err) {
                  // console.log('parent file hash', JSON.stringify({parentFile, hashValue}));
                  accept(hashValue);
                } else {
                  reject(err);
                }
              });
            }),
          ]).then(([key, parentKey]) => {
            const name = path.basename(file);
            // const key = murmurhash.v3(file, '');
            // const parentKey = murmurhash.v3(parentFile, '');
            const type = FILE;
            const {size} = stat;

            assetStats.push({
              path: file,
              name,
              key,
              parentKey,
              type,
              size,
              // blksize,
            });
          }).then(accept, reject);
        } else {
          reject(err);
        }
      });
    }))).then(accept, reject);
  });
}).then(() => {
  assetStats = assetStats.sort((a, b) => a.path.localeCompare(b.path));

  console.log('AssetStat *assetStats;');
  console.log('size_t numAssetStats;');
  console.log('void initAssetStats() {');
  console.log(`  assetStats = (AssetStat *)malloc(sizeof(AssetStat) * ${assetStats.length});`);
  console.log(`  numAssetStats = ${assetStats.length};`);
  // console.log(` printf("initialize asset stats %d", numAssetStats); fflush(stdout);`);
  for (let i = 0; i < assetStats.length; i++) {
    const assetStat = assetStats[i];
    console.log(`  { // ${assetStat.path}`);
    console.log(`    AssetStat *as = &assetStats[${i}];`);
    console.log(`    as->name = "${assetStat.name}";`);
    console.log(`    as->key = 0x${assetStat.key.toString(16)};`);
    console.log(`    as->parentKey = 0x${assetStat.parentKey.toString(16)};`);
    console.log(`    as->size = ${assetStat.size};`);
    console.log(`  }`);
    // console.log(`  AssetStat("${assetStat.name}", 0x${assetStat.key.toString(16)}, 0x${assetStat.parentKey.toString(16)}, ${assetStat.size}), // ${assetStat.path}`);
  }
  console.log('};\n');
}).catch(err => {
  console.warn(err.stack);
});
