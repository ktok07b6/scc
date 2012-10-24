#ifndef __IR_HPP__
#define __IR_HPP__

class IR;
class IRExp : public IR;
class IRStm : public IR;

class IntExp : public IRExp;
class UnaryExp : public IRExp;
class BinaryExp : public IRExp;
class CallExp : public IRExp;
class MemExp : public IRExp;
class TempExp : public IRExp;
class MoveExp : public IRStm;

class ExpStm : public IRStm;
class CJump : public IRStm;
class Jump : public IRStm;
class Loop : public IRStm;
class Label : public IRStm;
class Seq : public IRStm;

#endif
