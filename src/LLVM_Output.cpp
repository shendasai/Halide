#include "LLVM_Headers.h"
#include "LLVM_Output.h"
#include "CodeGen_LLVM.h"
#include "CodeGen_C.h"

#include <iostream>
#include <fstream>

using namespace std;

namespace Halide {

llvm::raw_fd_ostream *new_raw_fd_ostream(const std::string &filename) {
    std::string error_string;
#if LLVM_VERSION < 35
    llvm::raw_fd_ostream *raw_out = new llvm::raw_fd_ostream(filename.c_str(), error_string);
#elif LLVM_VERSION == 35
    llvm::raw_fd_ostream *raw_out = new llvm::raw_fd_ostream(filename.c_str(), error_string, llvm::sys::fs::F_None);
#else // llvm 3.6
    std::error_code err;
    llvm::raw_fd_ostream *raw_out = new llvm::raw_fd_ostream(filename.c_str(), err, llvm::sys::fs::F_None);
    if (err) error_string = err.message();
#endif
    internal_assert(error_string.empty())
        << "Error opening output " << filename << ": " << error_string << "\n";

    return raw_out;
}

void get_target_options(const llvm::Module *module, llvm::TargetOptions &options, std::string &mcpu, std::string &mattrs) {
    options = llvm::TargetOptions();
    options.LessPreciseFPMADOption = true;
    options.NoFramePointerElim = false;
    options.AllowFPOpFusion = llvm::FPOpFusion::Fast;
    options.UnsafeFPMath = true;
    options.NoInfsFPMath = true;
    options.NoNaNsFPMath = true;
    options.HonorSignDependentRoundingFPMathOption = false;
    options.UseSoftFloat = false;
    options.NoZerosInBSS = false;
    options.GuaranteedTailCallOpt = false;
    options.DisableTailCalls = false;
    options.StackAlignmentOverride = 0;
    options.TrapFuncName = "";
    options.PositionIndependentExecutable = true;
    options.UseInitArray = false;
    // TODO: Get these from metadata in the llvm::Module
    bool use_soft_float_abi = false;
    options.FloatABIType =
        use_soft_float_abi ? llvm::FloatABI::Soft : llvm::FloatABI::Hard;
    mcpu = "";
    mattrs = "";
}

llvm::TargetMachine *get_target_machine(const llvm::Module *module) {
    string error_string;

    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), error_string);
    if (!target) {
        cout << error_string << endl;
        llvm::TargetRegistry::printRegisteredTargetsForVersion();
    }
    internal_assert(target) << "Could not create target\n";

    llvm::TargetOptions options;
    std::string mcpu = "";
    std::string mattrs = "";
    get_target_options(module, options, mcpu, mattrs);

    return target->createTargetMachine(module->getTargetTriple(),
                                       mcpu, mattrs,
                                       options,
                                       llvm::Reloc::PIC_,
                                       llvm::CodeModel::Default,
                                       llvm::CodeGenOpt::Aggressive);
}

void emit_file(llvm::Module *module, const std::string &filename, llvm::TargetMachine::CodeGenFileType file_type) {
    Internal::debug(1) << "Compiling to native code...\n";
    Internal::debug(2) << "Target triple: " << module->getTargetTriple() << "\n";

    // Get the target specific parser.
    llvm::TargetMachine *target_machine = get_target_machine(module);
    internal_assert(target_machine) << "Could not allocate target machine!\n";

    // Build up all of the passes that we want to do to the module.
    llvm::PassManager pass_manager;

    // Add an appropriate TargetLibraryInfo pass for the module's triple.
    pass_manager.add(new llvm::TargetLibraryInfo(llvm::Triple(module->getTargetTriple())));

#if LLVM_VERSION < 33
    pass_manager.add(new llvm::TargetTransformInfo(target_machine->getScalarTargetTransformInfo(),
                                                   target_machine->getVectorTargetTransformInfo()));
#else
    target_machine->addAnalysisPasses(pass_manager);
#endif

#if LLVM_VERSION < 35
    llvm::DataLayout *layout = new llvm::DataLayout(module);
    Internal::debug(2) << "Data layout: " << layout->getStringRepresentation();
    pass_manager.add(layout);
#endif

    // Make sure things marked as always-inline get inlined
    pass_manager.add(llvm::createAlwaysInlinerPass());

    // Override default to generate verbose assembly.
    target_machine->setAsmVerbosityDefault(true);

    llvm::raw_fd_ostream *raw_out = new_raw_fd_ostream(filename);
    llvm::formatted_raw_ostream *out = new llvm::formatted_raw_ostream(*raw_out);

    // Ask the target to add backend passes as necessary.
    target_machine->addPassesToEmitFile(pass_manager, *out, file_type);

    pass_manager.run(*module);

    delete out;
    delete raw_out;

    delete target_machine;
}

llvm::Module *output_llvm_module(const Module &module) {
    return codegen_llvm(module);
}

void output_object(llvm::Module *module, const std::string &filename) {
    emit_file(module, filename, llvm::TargetMachine::CGFT_ObjectFile);
}

void output_assembly(llvm::Module *module, const std::string &filename) {
    emit_file(module, filename, llvm::TargetMachine::CGFT_AssemblyFile);
}

void output_native(llvm::Module *module,
                   const std::string &object_filename,
                   const std::string &assembly_filename) {
    emit_file(module, object_filename, llvm::TargetMachine::CGFT_ObjectFile);
    emit_file(module, assembly_filename, llvm::TargetMachine::CGFT_AssemblyFile);
}

void output_bitcode(llvm::Module *module, const std::string &filename) {
    llvm::raw_fd_ostream *file = new_raw_fd_ostream(filename);
    WriteBitcodeToFile(module, *file);
    delete file;
}

void output_llvm_assembly(llvm::Module *module, const std::string &filename) {
    llvm::raw_fd_ostream *file = new_raw_fd_ostream(filename);
    module->print(*file, NULL);
    delete file;
}

}  // namespace Halide
