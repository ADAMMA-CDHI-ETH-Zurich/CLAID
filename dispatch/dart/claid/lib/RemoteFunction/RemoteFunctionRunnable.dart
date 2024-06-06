import 'dart:mirrors';

import 'package:claid/generated/claidservice.pb.dart';


class RemoteFunctionRunnable<T>
{
  Function function;
  T returnTypeExample;
  List<dynamic> parameterTypes; 

  RemoteFunctionRunnable(
      this.function, this.returnTypeExample, this.parameterTypes) 
  {

  }

  DataPackage executeRemoteFunctionRequest(
      Object object, DataPackage rpcRequest) 
  {
    RemoteFunctionRunnableResult status;

    RemoteFunctionRequest executionRequest =
        rpcRequest.control_val.remote_function_request;

    RemoteFunctionIdentifier remoteFunctionIdentifier =
        executionRequest.remote_function_identifier;

    int payloadsSize = executionRequest.parameter_payloads.length;

    if (payloadsSize != this.parameterTypes.length) 
    {
      Logger.logError(
          'Failed to execute RemoteFunctionRunnable "${getFunctionSignature(remoteFunctionIdentifier, executionRequest)}". Number of parameters do not match. Function expected ${parameterTypes.length} parameters, but was executed with $payloadsSize');

      status = RemoteFunctionRunnableResult.makeFailedResult(
          RemoteFunctionStatus.FAILED_INVALID_NUMBER_OF_PARAMETERS);
      return makeRPCResponsePackage(status, rpcRequest);
    }

    List<Object> parameters = [];
    for (int i = 0; i < payloadsSize; i++) {
      Mutator mutator = TypeMapping().getMutator(this.parameterTypes[i]);

      DataPackage tmpPackage = DataPackage();
      tmpPackage.payload = executionRequest.parameter_payloads[i];

      Object data = mutator.getPackagePayload(tmpPackage);
      parameters.add(data);

      if (parameters[i].runtimeType != this.parameterTypes[i].runtimeType) 
      {
        Logger.logError(
            'Failed to execute RemoteFunctionRunnable "${getFunctionSignature(remoteFunctionIdentifier, executionRequest)}". Parameter object $i is of type "${parameters[i].runtimeType}", but expected type "${parameterTypes[i]}".');
        status = RemoteFunctionRunnableResult.makeFailedResult(
            RemoteFunctionStatus.FAILED_MISMATCHING_PARAMETERS);
        return makeRPCResponsePackage(status, rpcRequest);
      }
    }

    status = executeRemoteFunctionRequest(object, parameters);

    return makeRPCResponsePackage(status, rpcRequest);
  }

  RemoteFunctionRunnableResult executeRemoteFunctionRequest(
      List<Object> parameters) 
  {


  }

  static MethodMirror? lookupMethod(
      Object object, String functionName, List<Type> parameterTypes) {
    try {
      ClassMirror myClass = reflect(object).type;
      List<DeclarationMirror> declarations = myClass.declarations.values.toList();
      MethodMirror? method = declarations.firstWhereOrNull((mirror) {
        if (mirror is MethodMirror) {
          if (mirror.simpleName == Symbol(functionName)) {
            List<Type> params =
                mirror.parameters.map((p) => p.type.reflectedType).toList();
            return params.toString() == parameterTypes.toString();
          }
        }
        return false;
      }) as MethodMirror?;
      return method;
    } catch (e) {
      print(e);
      Logger.logError(
          'Failed to lookup function Method "$functionName" of object of type "${reflect(object).type}". Got exception "${e.toString()}".');
      return null;
    }
  }



  String getFunctionSignature(
      RemoteFunctionIdentifier remoteFunctionIdentifier,
      RemoteFunctionRequest remoteFunctionRequest) {
    String returnTypeName =
        returnType == null ? 'void' : MirrorSystem.getName(reflectType(returnType).simpleName);

    bool isRuntimeFunction = remoteFunctionIdentifier.hasRuntime();

    String parameterNames =
        parameterTypes.isNotEmpty ? MirrorSystem.getName(reflectType(parameterTypes[0]).simpleName) : '';

    for (int i = 1; i < parameterTypes.length; i++) {
      parameterNames +=
          ', ' + MirrorSystem.getName(reflectType(parameterTypes[i]).simpleName);
    }

    String functionSignature =
        isRuntimeFunction ? 'RuntimeFunction: ' : 'ModuleFunction: ';
    functionSignature +=
        '$returnTypeName $functionName ($parameterNames)';

    return functionSignature;
  }

  RemoteFunctionReturn makeRemoteFunctionReturn(
      RemoteFunctionRunnableResult result, RemoteFunctionRequest executionRequest) {
    RemoteFunctionReturn remoteFunctionReturn = RemoteFunctionReturn();

    RemoteFunctionIdentifier remoteFunctionIdentifier =
        executionRequest.remoteFunctionIdentifier;

    remoteFunctionReturn.executionStatus = result.status;
    remoteFunctionReturn.remoteFunctionIdentifier = remoteFunctionIdentifier;
    remoteFunctionReturn.remoteFutureIdentifier =
        executionRequest.remoteFutureIdentifier;

    return remoteFunctionReturn;
  }

  DataPackage makeRPCResponsePackage(
      RemoteFunctionRunnableResult result, DataPackage rpcRequest) {
    RemoteFunctionRequest executionRequest =
        rpcRequest.controlVal.remoteFunctionRequest;

    DataPackage responsePackage = DataPackage();

    responsePackage.sourceModule = rpcRequest.targetModule;
    responsePackage.targetModule = rpcRequest.sourceModule;

    ControlPackage ctrlPackage = ControlPackage();
    ctrlPackage.ctrlType = CtrlType.CTRL_REMOTE_FUNCTION_RESPONSE;
    ctrlPackage.remoteFunctionReturn =
        makeRemoteFunctionReturn(result, executionRequest);

    ctrlPackage.runtime = Runtime.RUNTIME_JAVA;

    responsePackage.controlVal

      Object returnValue = result.getReturnValue();
      if(returnValue != null)
      {
          Mutator<?> mutator = TypeMapping.getMutator(new DataType(this.returnType));
          responsePackage = mutator.setPackagePayloadFromObject(responsePackage, returnValue);
      }

      return responsePackage;
  }
}
