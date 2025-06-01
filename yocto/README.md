# Yocto Build Setup

This directory contains a minimal configuration for building the `rust-spray-image` using Poky. The workflow sources `poky/oe-init-build-env` and then runs BitBake.

The `bblayers.conf` enables standard Poky layers along with the `meta-rust-spray` layer included in this repository.

To build locally:

```bash
source poky/oe-init-build-env build
bitbake rust-spray-image
```
