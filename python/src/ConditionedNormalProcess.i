// SWIG file ConditionedNormalProcess.i

%{
#include "openturns/ConditionedNormalProcess.hxx"
%}

%include ConditionedNormalProcess_doc.i

%include openturns/ConditionedNormalProcess.hxx
namespace OT { %extend ConditionedNormalProcess { ConditionedNormalProcess(const ConditionedNormalProcess & other) { return new OT::ConditionedNormalProcess(other); } } }
