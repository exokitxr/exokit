var args = process.argv.slice(2);
if (args[0].includes("exokit://")){
  result = args[0].replace(/(^\w+:|^)\/\/\w+\/\//, '');
  result = 'https://' + result;
  console.log(result);
} else {
  console.log(args[0]);
}
