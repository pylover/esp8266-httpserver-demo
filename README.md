
# Helloword 

### Build

Follow [this](https://github.com/pylover/esp8266-env) instruction 
to setup your environment.

Connect an esp8266 board to your PC.

```bash
cd esp8266-env
source activate.sh
cd ..

cd esp8266-httpserver-demo
bash gen_misc.sh
```

Or use predefined make macros:

```bash
make clean
make assets_map6user1
make flash_map6user1 

```

### Tests

```bash
make test
```

```
TODO: Readme
TODO: Sequence diagrams
TODO: Speed optimize
```
