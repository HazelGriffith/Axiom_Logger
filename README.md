# Axiom Logger Project

## Introduction
This is a project for developing a Cadmium Logger that generates (Thousands of Problems for Theorem Provers) TPTP compatible logical axioms in Typed First-Order Logic (TFF) for checking if an error exists during a simulation using DEVS-EP.

## Dependencies
This project assumes that you have Cadmium installed in a location accessible by the environment variable $CADMIUM.
_This dependency would be met by default if you are using the ARSLAB servers. To check, try `echo $CADMIUM` in the terminal_

## Build
To build this project, run:
```sh
source build_sim.sh
```

## Execute
To run the project, run:
```sh
./bin/Axiom_Logger
```

## Short-term Goals
The end goal of this project is to make running DEVS-EP during Cadmium simulations compatible with any DEVS model.

- [&check;] Finish Start method
    - [&check;] start csv file stream
    - [&check;] store model variable and port info from DEVSMap files
    - [&check;] make it compatible with duplicate models
- [ ] Finish LogModel method
- [ ] Finish LogState method
- [ ] Finish LogOutput/Input methods

## Long-term Goals

- [ ] Add Coupled Model Axioms
- [ ] Add general ATP support
- [ ] Add custom conjectures support