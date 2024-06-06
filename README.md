# Spiking Neural Network in C++
To run the network:
- Clone the repository

```bash
git clone -b event https://github.com/samp5/ha141.git

```

- Create the following directories in the root folder of the repo
```bash
mkdir build logs
```

- Build the executable
```bash
make build
```

- Run the network
```bash
make run
```

If it doesn't already exist this will create a `base_config.toml` file in `run_config/`. If you want to run the network on a different config use `build/snn <filename>.toml`
```bash
build/snn my_custom_config.toml
```
All other options are specified in config file
