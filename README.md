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

