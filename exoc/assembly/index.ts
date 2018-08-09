// The entry file of your WebAssembly module.

declare namespace console {
  function logi(val: i32): void;
}

export function add(a: i32, b: i32): i32 {
  return a + b; //lol
}

export function main(): i32 {
  console.logi(0);
  // console.log('lol');
  // prints('lol');
  return 0;
}
