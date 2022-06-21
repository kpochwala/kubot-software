[![Build](https://github.com/kpochwala/kubot-software/actions/workflows/build.yml/badge.svg)](https://github.com/kpochwala/kubot-software/actions/workflows/build.yml)

# Zephyr Kubot Minisumo Software
## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment. You can follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Initialization

The first step is to initialize the workspace folder (``kubot-workspace``) where
the ``kubot-software`` and all Zephyr modules will be cloned. You can do
that by running:

```shell
# initialize my-workspace for the example-application (main branch)
west init -m git@github.com:kpochwala/kubot-software.git --mr main kubot-workspace
# update Zephyr modules
cd kubot-workspace
west update
```

### Build & Run

The application can be built by running:

```shell
west build -b kubot_f765vi -s app
```
And flashed:

```shell
west flash
```
