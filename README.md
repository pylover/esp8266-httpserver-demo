# esp8266-httpserver-demo
https://github.com/pylover/esp8266-httpserver demo project




### Build

Follow [this](https://github.com/pylover/esp8266-env) instruction 
to setup your environment.


```bash
cd myenv
source activate.sh

git clone --recursive git@github.com:pylover/esp8266-httpserver-demo.git demo
cd demo

make clean
make assets_map6user1
make flash_map6user1 

```

Or:

```bash
./gen_misc.sh
```
