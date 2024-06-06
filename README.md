# Spiking Neural Network in C++
To run the network:
- Clone the repository

```bash
git clone -b event https://github.com/samp5/ha141.git

```

- Create the following directories in the root folder of the repo
```bash
mkdir build input_files logs
```

- Build the executable
```bash
make build
```

- Run the network
```
make run
```

This will create a base_config.toml file in run_config/. If you want to run the network on a different config use `build/snn <filename>.toml`
```bash
build/snn my_custom_config.toml

```

