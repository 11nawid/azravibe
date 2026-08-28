# Native C Extensions

Azravibe can load native modules from the import path. A native module is a `.dll` on Windows, `.so` on Linux, or `.dylib` on macOS that exports:

```c
AZR_NATIVE_EXPORT int azr_module_init(Environment *module_env, const AzrNativeAPI *api);
```

Inside `azr_module_init`, register functions into `module_env`.

```c
#include "src/azravibe_native.h"

static const AzrNativeAPI *azr = 0;

static Value *square(Value **args, int argc) {
    long long n = 0;
    if (argc > 0 && args[0]) {
        n = azr->value_as_int(args[0]);
    }
    return azr->make_int(n * n);
}

AZR_NATIVE_EXPORT int azr_module_init(Environment *module_env, const AzrNativeAPI *api) {
    if (!module_env || !api || api->abi_version != AZR_NATIVE_ABI_VERSION) return 0;
    azr = api;
    api->env_set(module_env, "square", api->make_builtin("square", square));
    return 1;
}
```

Compile it next to your Azravibe script or inside a project import path.

Windows example:

```powershell
gcc -shared -O2 -I. -o native_math.dll examples/native_math.c
```

Azravibe usage:

```azr
بیار native_math
بنویس(native_math.square(9))
```

Native modules are for performance-heavy code and OS bindings. Normal libraries should be written in `.azr` first.
