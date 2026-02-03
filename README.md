# ZMK Battery Percentage Typing Behavior

Types the keyboard battery percentage as digits when triggered (e.g. `87` or `100`). Digits only.

## Installation

### 1. Add as a ZMK module

In your **zmk-config** repo, edit `config/west.yml`:

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: eintim
      url-base: https://github.com/eintim

  projects:
    - name: zmk
      remote: zmkfirmware
      revision: main
      import: app/west.yml

    - name: zmk-behavior-battery-typer
      remote: eintim
      revision: main

  self:
    path: config
```

### 2. Include the behavior in your keymap

At the top of your keymap file:

```dts
#include <behaviors/bat_print.dtsi>
```

Bind it like a normal behavior:

```dts
&bat_print
```

### 3. Optional: Bind to a combo

```dts
/ {
  combos {
    compatible = "zmk,combos";

    batt_combo {
      timeout-ms = <40>;
      key-positions = <12 13>;  // adjust for your layout
      bindings = <&bat_print>;
    };
  };
};
```

## Split keyboards

To type both sides (e.g. `87 92`), set in your config:

`CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y`

Otherwise only the central side is typed.
