# AAOS Skins

Each skin demonstrates different device configuration.

## Use skin
1. locate `<andorid_patch>/out/target/product/emulator_car64_x86_64/config.ini`, path might change depends on board name.
2. find below code
```
skin.name=1408x792
skin.path=1408x792
### automotive_landscape start
; skin.name = automotive_landscape
; skin.path = <andorid_patch>/device/generic/car/emulator/skins/automotive_landscape
; hw.initialOrientation = landscape
### automotive_landscape end

### automotive_large_portrait start
; skin.name = automotive_large_portrait
; skin.path = <andorid_patch>/device/generic/car/emulator/skins/automotive_large_portrait
; hw.initialOrientation = portrait
### automotive_large_portrait end

### automotive_ultrawide start
; skin.name = automotive_ultrawide
; skin.path = <andorid_patch>/device/generic/car/emulator/skins/automotive_ultrawide
; hw.initialOrientation = landscape
### automotive_ultrawide end
```
3. comment out the original skin
```
; skin.name=1408x792
; skin.path=1408x792
```
4. Un-comment desiered skin
```
### automotive_ultrawide start
 skin.name = automotive_ultrawide
 skin.path = <andorid_patch>/device/generic/car/emulator/skins/automotive_ultrawide
 hw.initialOrientation = landscape
### automotive_ultrawide end
```
5. launch emulator again with wipe-data flag
```
emulator -wipe-data
```