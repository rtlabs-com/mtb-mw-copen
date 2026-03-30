
# C-Open Middleware
<img src="img/c-open.svg" alt="C-Open logo" width="100" style="float:right; margin-right:50px;margin-bottom: 50px">

## Introduction to C-Open

C-Open is a CANopen CC (classic) protocol stack implementation that enables developers to build robust CANopen master and slave devices for industrial automation systems. The stack provides an implementation of the CANopen communication profile (CiA 301) and Layer Setting Services (LSS, CiA 305), making it suitable for a wide range of industrial applications including factory automation, motion control, and embedded systems.

Designed with flexibility and portability in mind, C-Open features an operating system abstraction layer (OSAL) that allows it to run on any real-time operating system (RTOS), or general-purpose operating systems such as Linux and Windows. This architecture ensures that your CANopen application can be deployed across different hardware platforms with minimal code changes.

The stack is optimized for resource-constrained embedded systems with a minimal memory footprint (approximately 15 KB ROM and less than 400 bytes RAM on Cortex-M4, plus user-defined object dictionary storage), while maintaining full protocol compliance and high performance.

## Key features:

- **Dual Role Support**: Implement both CANopen master and slave devices using the same stack
- **Network Management (NMT)**: Full support for network initialization, configuration, and state management
- **Service Data Objects (SDO)**: Expedited and segmented transfers for configuration and parameter access
- **Process Data Objects (PDO)**: High-performance real-time data exchange with configurable mapping
- **Emergency Objects (EMCY)**: Error reporting and diagnostic capabilities
- **Heartbeat and Node Guarding**: Network monitoring and fault detection mechanisms
- **Layer Setting Services (LSS)**: Dynamic node-ID assignment and bitrate configuration (CiA 305)
- **Multi-Instance Support**: Run multiple CANopen networks simultaneously
- **Portable Architecture**: Written to a OS abstraction layer
- **Minimal Footprint**: Optimized for resource-constrained embedded systems
- **Production Ready**: Tested with CANopen Conformance Test Tool

# Limitations

The following features are currently not implemented:

- **SDO block transfer**
- **MPDO**
- **CANopen FD**

Note that the stack can be used with a CAN FD controller in classic mode, but does not yet support CANopen FD.

## C-Open for Modus Toolbox

This library package is an adaptation of C-Open for Modus Toolbox and the XMC72_EVK platform.

- [C-Open User Example](https://github.com/rtlabs-com/mtb-example-copen) - C-Open MTB Example Application on GitHub
- [C-Open Middleware](https://github.com/rtlabs-com/mtb-mw-copen) - C-Open MTB Middleware on GitHub
- [C-Open](https://rt-labs.com/product/c-open) - C-Open product information

### Time limitation

Runtime of C-Open library is limited to 2 hours. To obtain the full version, please contact your regional sales representative of Infineon Technologies AG.

### Documentation

- [C-Open](https://docs.rt-labs.com/c-open) (user account & login required)

### License

[LICENSE](./LICENSE.md)
