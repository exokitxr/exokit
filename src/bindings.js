// Set by exokit.setNativeBindingsModule.

// Due to the way `require` works with multithreading (used for `Worker`), we use this
// route to make sure the binaries only get initialized once.
// This might change with node's native "worker" support that's coming.
