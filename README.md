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

The behavior requires battery reporting (`CONFIG_ZMK_BATTERY_REPORTING`), which is on by default for BLE keyboards.

### Optional: Adjust typing speed

Keys are typed through ZMK's behavior queue, the same mechanism macros use. If characters get dropped on your host, slow it down:

```dts
&bat_print {
  tap-ms = <30>;   // how long each key is held (default 30)
  wait-ms = <15>;  // pause after each key (default 15)
};
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

## ZMK Studio

The behavior shows up in [ZMK Studio](https://zmk.dev/docs/features/studio) as **Battery Print**. Studio builds keep it in the firmware even when your keymap doesn't reference it, as long as the keymap includes `behaviors/bat_print.dtsi`, so you can assign it to a key from Studio. To leave it out, define `ZMK_BEHAVIORS_OMIT_BAT_PRINT` before the include.

## Split keyboards

To also type the peripheral halves (e.g. `87 92`), set in your config:

`CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y`

Otherwise only the central side is typed. With multiple peripherals, each level is typed in order, separated by spaces. A peripheral that hasn't reported a level yet shows as `0`.
