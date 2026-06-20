// =============================================================================
// visitor.cpp — Implementación de TypeCheckerVisitor y GenCodeVisitor
// =============================================================================

#include "visitor.h"
#include "ast.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

// =============================================================================
// Despacho del patrón Visitor (accept en cada nodo del AST)
// =============================================================================

int BinaryExp::accept(Visitor *v) { return v->visit(this); }
int NumberExp::accept(Visitor *v) { return v->visit(this); }
int IdExp::accept(Visitor *v) { return v->visit(this); }
int IdExp::computeAddress(Visitor *v) { return v->computeAddress(this); }
int Program::accept(Visitor *v) { return v->visit(this); }
int PrintStm::accept(Visitor *v) { return v->visit(this); }
int AssignStm::accept(Visitor *v) { return v->visit(this); }
int ExpListSize::accept(Visitor *v) { return v->visit(this); }
int ExpListVals::accept(Visitor *v) { return v->visit(this); }
int IfStm::accept(Visitor *v) { return v->visit(this); }
int WhileStm::accept(Visitor *v) { return v->visit(this); }
int Body::accept(Visitor *v) { return v->visit(this); }
int VarDec::accept(Visitor *v) { return v->visit(this); }
int FcallExp::accept(Visitor *v) { return v->visit(this); }
int FunDec::accept(Visitor *v) { return v->visit(this); }
int ReturnStm::accept(Visitor *v) { return v->visit(this); }
int DoWhileStm::accept(Visitor *v) { return v->visit(this); }
int BreakStm::accept(Visitor *v) { return v->visit(this); }
int SwitchStm::accept(Visitor *v) { return v->visit(this); }
int UnaryExp::accept(Visitor *v) { return v->visit(this); }
int IndexExp::accept(Visitor *v) { return v->visit(this); }
int IndexExp ::computeAddress(Visitor *v) { return v->computeAddress(this); }
int StructExp::accept(Visitor *v) { return v->visit(this); }
int FieldAccessExp::accept(Visitor *v) { return v->visit(this); }
int FieldAccessExp::computeAddress(Visitor *v) { return v->computeAddress(this); }
int MatrixSizeExp::accept(Visitor *v) { return v->visit(this); }
int MatrixIndexExp::accept(Visitor *v) { return v->visit(this); }
int MatrixIndexExp::computeAddress(Visitor *v) { return v->computeAddress(this); }
int MatrixValsExp::accept(Visitor *v) { return v->visit(this); }
int StructDec::accept(Visitor *v) { return v->visit(this); }

// =============================================================================
// Implementación de accept()
// =============================================================================

// =============================================================================
// TypeCheckerVisitor — Análisis semántico
// =============================================================================
// Responsabilidades:
//   1. Registrar las funciones y su aridad antes de analizar los cuerpos.
//   2. Verificar que cada variable usada haya sido declarada en el scope.
//   3. Verificar que cada función llamada exista y reciba el número correcto
//      de argumentos.
//   4. Contar las variables locales de cada función para que GenCodeVisitor
//      pueda reservar el espacio correcto en el stack frame.
// =============================================================================

// -----------------------------------------------------------------------------
// TypeChecker — punto de entrada
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::TypeChecker(Program *program) {
  // Primera pasada: registrar todas las funciones y su aridad.
  // Esto permite llamadas hacia adelante (funciones que se usan antes de
  // ser definidas en el orden textual del fuente).
  for (auto fd : program->fdlist) {
    funAridad[fd->nombre] = static_cast<int>(fd->Pnombres.size());
  }

  // Segunda pasada: analizar cada función
  for (auto fd : program->fdlist) {
    fd->accept(this);
  }

  return 0;
}

// -----------------------------------------------------------------------------
// visit(FunDec) — analiza una declaración de función
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(FunDec *fd) {
  funcionActual = fd->nombre;
  locales = 0;
  int parametros = static_cast<int>(fd->Pnombres.size());

  // Abrir scope de la función
  entorno.add_level();

  // Registrar los parámetros como variables del scope actual
  for (auto &nombre : fd->Pnombres)
    entorno.add_var(nombre, 0);

  // Analizar el cuerpo
  fd->cuerpo->accept(this);

  // Cerrar scope
  entorno.remove_level();

  // Guardar el tamaño total del frame (parámetros + locales)
  funcontador[fd->nombre] = parametros + locales;
  return 0;
}

// -----------------------------------------------------------------------------
// visit(Body) — analiza declaraciones y sentencias del cuerpo
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(Body *body) {
  entorno.add_level();

  for (auto dec : body->declarations)
    dec->accept(this);
  for (auto stm : body->StmList)
    stm->accept(this);

  entorno.remove_level();
  return 0;
}

// -----------------------------------------------------------------------------
// visit(VarDec) — registra las variables declaradas y cuenta los locales
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(VarDec *vd) {
  for (auto &nombre : vd->vars) {
    if (entorno.check(nombre)) {
      std::cerr << "[TypeChecker] Advertencia: la variable '" << nombre
                << "' ya fue declarada en este scope"
                << " (en función '" << funcionActual << "').\n";
    }
    entorno.add_var(nombre, 0);
    locales++;
  }
  return 0;
}

// -----------------------------------------------------------------------------
// visit(IdExp) — verifica que la variable esté declarada
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(IdExp *exp) {
  if (!entorno.check(exp->value)) {
    throw std::runtime_error("[TypeChecker] Variable no declarada: '" +
                             exp->value + "' usada en la función '" +
                             funcionActual + "'");
  }
  return 0;
}

// -----------------------------------------------------------------------------
// (IdExp) — verifica que la variable esté declarada
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::computeAddress(IdExp *exp) { return 0; }

// -----------------------------------------------------------------------------
// visit(AssignStm) — verifica variable y evalúa expresión
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(AssignStm *stm) { return 0; }

// -----------------------------------------------------------------------------
// visit(ExpListSize) — verifica variable y evalúa expresión
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(ExpListSize *StmListSize) { return 0; }

// -----------------------------------------------------------------------------
// visit(ExpListVals) — verifica variable y evalúa expresión
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(ExpListVals *StmListVals) { return 0; }

// -----------------------------------------------------------------------------
// visit(FcallExp) — verifica existencia de la función y aridad
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(FcallExp *fcall) {
  // Verificar que la función exista
  if (funAridad.find(fcall->nombre) == funAridad.end()) {
    throw std::runtime_error("[TypeChecker] Función no definida: '" +
                             fcall->nombre + "' llamada en '" + funcionActual +
                             "'");
  }

  // Verificar número de argumentos
  int esperados = funAridad[fcall->nombre];
  int recibidos = static_cast<int>(fcall->argumentos.size());
  if (recibidos != esperados) {
    throw std::runtime_error("[TypeChecker] La función '" + fcall->nombre +
                             "' espera " + std::to_string(esperados) +
                             " argumento(s), pero se pasaron " +
                             std::to_string(recibidos));
  }

  // Verificar cada argumento
  for (auto arg : fcall->argumentos)
    arg->accept(this);

  return 0;
}

// -----------------------------------------------------------------------------
// visit(IfStm) — cuenta máximo de locales entre ramas
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(IfStm *stm) {
  stm->condition->accept(this);

  int base = locales;

  // Rama then
  locales = 0;
  stm->then->accept(this);
  int maxLocales = locales;

  // Rama else (opcional)
  if (stm->els) {
    locales = 0;
    stm->els->accept(this);
    maxLocales = std::max(maxLocales, locales);
  }

  // El frame necesita el máximo de ambas ramas
  locales = base + maxLocales;
  return 0;
}

// -----------------------------------------------------------------------------
// visit(WhileStm)
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(WhileStm *stm) {
  stm->condition->accept(this);
  stm->b->accept(this);
  return 0;
}

// -----------------------------------------------------------------------------
// visit(PrintStm)
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(PrintStm *stm) {
  stm->e->accept(this);
  return 0;
}

// -----------------------------------------------------------------------------
// visit(ReturnStm)
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(ReturnStm *r) {
  r->e->accept(this);
  return 0;
}

// -----------------------------------------------------------------------------
// Nodos que no requieren verificación semántica adicional
// -----------------------------------------------------------------------------

int TypeCheckerVisitor::visit(BinaryExp *exp) {
  exp->left->accept(this);
  exp->right->accept(this);
  return 0;
}

int TypeCheckerVisitor::visit(UnaryExp *exp) {
  exp->operand->accept(this);
  return 0;
}
int TypeCheckerVisitor::visit(IndexExp *exp) {
  // TODO
  return 0;
}

int TypeCheckerVisitor::computeAddress(IndexExp *exp) { return 0; }

int TypeCheckerVisitor::visit(NumberExp *exp) { return 0; }
int TypeCheckerVisitor::visit(Program *p) { return 0; }

int TypeCheckerVisitor::visit(DoWhileStm *stm) {
  stm->condition->accept(this);
  stm->b->accept(this);
  return 0;
}

int TypeCheckerVisitor::visit(BreakStm *stm) { return 0; }

int TypeCheckerVisitor::visit(SwitchStm *stm) {
  stm->e->accept(this);
  for (auto c : stm->cases) {
    for (auto s : c->body)
      s->accept(this);
  }
  for (auto s : stm->default_body)
    s->accept(this);
  return 0;
}

int TypeCheckerVisitor::visit(StructExp *exp) {
  for (auto v : exp->values)
    v->accept(this);
  return 0;
}

int TypeCheckerVisitor::visit(FieldAccessExp *exp) { return 0; }
int TypeCheckerVisitor::computeAddress(FieldAccessExp *exp) { return 0; }

int TypeCheckerVisitor::visit(MatrixSizeExp *exp) {
  exp->rows->accept(this);
  exp->cols->accept(this);
  return 0;
}

int TypeCheckerVisitor::visit(MatrixIndexExp *exp) { return 0; }
int TypeCheckerVisitor::computeAddress(MatrixIndexExp *exp) { return 0; }
int TypeCheckerVisitor::visit(MatrixValsExp *exp) { return 0; }
int TypeCheckerVisitor::visit(StructDec *sd) { return 0; }

// =============================================================================
// GenCodeVisitor — Generación de código ensamblador x86-64 (AT&T syntax)
// =============================================================================
// Convenciones usadas:
//   · Registros de argumentos: %rdi, %rsi, %rdx, %rcx, %r8, %r9
//   · Resultado de expresiones en %rax
//   · Variables locales: offsets negativos desde %rbp
//   · Variables globales: símbolos en .data con acceso RIP-relativo
//   · printf para print: formato en print_fmt ("%ld \n")
// =============================================================================

// -----------------------------------------------------------------------------
// generar — punto de entrada de la generación
// -----------------------------------------------------------------------------

int GenCodeVisitor::generar(Program *program) {
  tipos.TypeChecker(program);
  funcontador = tipos.funcontador;
  program->accept(this);
  return 0;
}

// -----------------------------------------------------------------------------
// visit(Program)
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(StructDec *sd) {
  structDefs[sd->name] = sd->fieldNames;
  return 0;
}

int GenCodeVisitor::visit(Program *program) {
  // Registrar definiciones de struct
  for (auto sd : program->sdlist)
    sd->accept(this);

  // Sección de datos
  out << ".data\n";
  out << "print_fmt: .string \"%ld \\n\"\n";

  // Recolectar nombres de variables globales
  for (auto dec : program->vdlist)
    dec->accept(this);

  // Emitir las etiquetas .quad para las globales
  for (auto &[var, _] : memoriaGlobal)
    out << var << ": .quad 0\n";

  // Sección de texto (código)
  out << "\n.text\n";

  for (auto fd : program->fdlist)
    fd->accept(this);

  // Marca el stack como no ejecutable (requerido por el linker moderno)
  out << "\n.section .note.GNU-stack,\"\",@progbits\n";
  return 0;
}

// -----------------------------------------------------------------------------
// visit(VarDec) — registra variables en memoria (global o local)
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(VarDec *stm) {
  // Registrar definición de estructura si tiene campos (inline struct)
  if (!stm->structFields.empty()) {
    structDefs[stm->type] = stm->structFields;
  }

  for (auto &var : stm->vars) {
    if (!entornoFuncion) {
      memoriaGlobal[var] = true;
    } else {
      memoria[var] = offset;
      offset -= 8;
    }
    // Si el tipo coincide con un struct conocido, registrar la asociación
    if (structDefs.count(stm->type)) {
      varStructType[var] = stm->type;
    }
  }
  return 0;
}

// -----------------------------------------------------------------------------
// visit(NumberExp) — carga un inmediato en %rax
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(NumberExp *exp) {
  out << "  movq $" << exp->value << ", %rax\n";
  return 0;
}

// -----------------------------------------------------------------------------
// visit(ExpListSize) — evalúa la expresión para reservar el espacio apropiado y
// dejarlo en rax
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(ExpListSize *stm) {
  stm->size->accept(this);
  out << "  movq $8, %rcx\n";
  out << "  imulq %rcx, %rax\n";
  out << "  movq %rax, %rdi\n"
      << "  call malloc@PLT\n";
  return 0;
}

// -----------------------------------------------------------------------------
// visit(ExpListVals) — evalúa las expresiones expresión y las almacena en los
// espacios reservados de memoria y luego regresa el puntero
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(ExpListVals *stm) {
  // Allocate like ExpListSize: n * 8 bytes
  int n = stm->values.size();
  out << "  movq $" << n << ", %rax\n";
  out << "  movq $8, %rcx\n";
  out << "  imulq %rcx, %rax\n";
  out << "  movq %rax, %rdi\n"
      << "  call malloc@PLT\n";
  // Pointer is in %rax, will be stored by AssignStm
  // Values will be stored by AssignStm post-processing
  return 0;
}

// -----------------------------------------------------------------------------
// visit(IdExp) — carga el valor de una variable en %rax
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(IdExp *exp) {
  if (memoriaGlobal.count(exp->value))
    out << "  movq " << exp->value << "(%rip), %rax\n";
  else
    out << "  movq " << memoria[exp->value] << "(%rbp), %rax\n";
  return 0;
}

// visit(UnaryExp) — operador unario NOT
int GenCodeVisitor::visit(UnaryExp *exp) {
  exp->operand->accept(this);
  int lbl = labelcont++;
  out << "  cmpq $0, %rax\n";
  out << "  je not_true_" << lbl << "\n";
  out << "  movq $0, %rax\n";
  out << "  jmp not_end_" << lbl << "\n";
  out << "not_true_" << lbl << ":\n";
  out << "  movq $1, %rax\n";
  out << "not_end_" << lbl << ":\n";
  return 0;
}

int GenCodeVisitor::visit(IndexExp *exp) {
  // 1) Evaluar el índice → %rax
  exp->index->accept(this);
  // 2) Índice a %rdi
  out << "  movq %rax, %rdi\n";
  // 3) Cargar el puntero base en %rax
  if (memoriaGlobal.count(exp->name))
    out << "  movq " << exp->name << "(%rip), %rax\n";
  else
    out << "  movq " << memoria[exp->name] << "(%rbp), %rax\n";
  // 4) Leer el elemento
  out << "  movq (%rax, %rdi, 8), %rax\n";
  return 0;
}

// -----------------------------------------------------------------------------
// visit(BinaryExp) — evalúa left y right, aplica operador
// Convención: left en %rax, right en %rcx
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(BinaryExp *exp) {
  exp->left->accept(this);
  out << "  pushq %rax\n";
  exp->right->accept(this);
  out << "  movq %rax, %rcx\n";
  out << "  popq %rax\n";

  switch (exp->op) {
  case PLUS_OP:
    out << "  addq %rcx, %rax\n";
    break;
  case MINUS_OP:
    out << "  subq %rcx, %rax\n";
    break;
  case MUL_OP:
    out << "  imulq %rcx, %rax\n";
    break;
  case DIV_OP:
    out << "  cqto\n";
    out << "  idivq %rcx\n";
    break;
  case LE_OP:
    out << "  cmpq %rcx, %rax\n";
    out << "  movq $0, %rax\n";
    out << "  setl %al\n";
    out << "  movzbq %al, %rax\n";
    break;
  case GT_OP:
    out << "  cmpq %rcx, %rax\n";
    out << "  movq $0, %rax\n";
    out << "  setg %al\n";
    out << "  movzbq %al, %rax\n";
    break;
  case LEQ_OP:
    out << "  cmpq %rcx, %rax\n";
    out << "  movq $0, %rax\n";
    out << "  setle %al\n";
    out << "  movzbq %al, %rax\n";
    break;
  case GEQ_OP:
    out << "  cmpq %rcx, %rax\n";
    out << "  movq $0, %rax\n";
    out << "  setge %al\n";
    out << "  movzbq %al, %rax\n";
    break;
  case EQ_OP:
    out << "  cmpq %rcx, %rax\n";
    out << "  movq $0, %rax\n";
    out << "  sete %al\n";
    out << "  movzbq %al, %rax\n";
    break;
  case NE_OP:
    out << "  cmpq %rcx, %rax\n";
    out << "  movq $0, %rax\n";
    out << "  setne %al\n";
    out << "  movzbq %al, %rax\n";
    break;
  case AND_OP:
    out << "  andq %rcx, %rax\n";
    break;
  case OR_OP:
    out << "  orq %rcx, %rax\n";
    break;
  }
  return 0;
}

// -----------------------------------------------------------------------------
// visit(AssignStm) — evalúa expresión y almacena resultado
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(AssignStm *stm) {
  stm->e->accept(this);
  stm->target->computeAddress(this);

  // Get target variable name
  IdExp *targetId = dynamic_cast<IdExp *>(stm->target);
  std::string targetName = targetId ? targetId->value : "";

  // Track matrix cols for static indexing
  if (auto *ms = dynamic_cast<MatrixSizeExp *>(stm->e)) {
    NumberExp *colsNum = dynamic_cast<NumberExp *>(ms->cols);
    if (colsNum && !targetName.empty())
      varMatrixCols[targetName] = colsNum->value;
  }

  // Post-init: store values for list/struct/matrix init expressions
  auto emitIndexStore = [&](const std::string &varName, int idx, Exp *val) {
    val->accept(this);
    out << "  pushq %rax\n";
    out << "  movq $" << idx << ", %rax\n";
    out << "  movq %rax, %rdi\n";
    out << "  popq %rax\n";
    out << "  movq %rax, %rcx\n";
    if (memoriaGlobal.count(varName))
      out << "  movq " << varName << "(%rip), %rax\n";
    else
      out << "  movq " << memoria[varName] << "(%rbp), %rax\n";
    out << "  movq %rcx, (%rax, %rdi, 8)\n";
  };

  if (auto *lv = dynamic_cast<ExpListVals *>(stm->e)) {
    for (size_t i = 0; i < lv->values.size(); ++i)
      emitIndexStore(targetName, i, lv->values[i]);
  } else if (auto *se = dynamic_cast<StructExp *>(stm->e)) {
    for (size_t i = 0; i < se->values.size(); ++i)
      emitIndexStore(targetName, i, se->values[i]);
  } else if (auto *mv = dynamic_cast<MatrixValsExp *>(stm->e)) {
    NumberExp *colsNum = dynamic_cast<NumberExp *>(mv->cols);
    int cols = colsNum ? colsNum->value : 1;
    if (!targetName.empty())
      varMatrixCols[targetName] = cols;
    for (size_t i = 0; i < mv->values.size(); ++i) {
      int row = i / cols;
      int col = i % cols;
      mv->values[i]->accept(this);
      out << "  pushq %rax\n";
      out << "  movq $" << row << ", %rax\n";
      out << "  pushq %rax\n";
      out << "  movq $" << col << ", %rax\n";
      out << "  movq %rax, %rdi\n";
      out << "  popq %rax\n";
      out << "  movq $" << cols << ", %rcx\n";
      out << "  imulq %rcx, %rax\n";
      out << "  addq %rdi, %rax\n";
      out << "  movq %rax, %rdi\n";
      out << "  popq %rcx\n";
      if (memoriaGlobal.count(targetName))
        out << "  movq " << targetName << "(%rip), %rax\n";
      else
        out << "  movq " << memoria[targetName] << "(%rbp), %rax\n";
      out << "  movq %rcx, (%rax, %rdi, 8)\n";
    }
  }
  return 0;
}

int GenCodeVisitor::computeAddress(IdExp *id) {
  if (memoriaGlobal.count(id->value)) {
    out << "  movq %rax, " << id->value << "(%rip)\n";
  } else {
    int off = memoria[id->value];
    out << "  movq %rax, " << off << "(%rbp)\n";
  }
  return 0;
}

int GenCodeVisitor::computeAddress(IndexExp *idx) {
  out << "  pushq %rax\n";
  idx->index->accept(this);
  out << "  movq %rax, %rdi\n"
      << "  popq %rax\n"
      << "  movq %rax, %rcx\n";
  if (memoriaGlobal.count(idx->name))
    out << "  movq " << idx->name << "(%rip), %rax\n";
  else
    out << "  movq " << memoria[idx->name] << "(%rbp), %rax\n";
  out << "  movq %rcx, (%rax, %rdi, 8)\n";
  return 0;
}

int GenCodeVisitor::visit(PrintStm *stm) {
  stm->e->accept(this);
  out << "  movq %rax, %rsi\n";
  out << "  leaq print_fmt(%rip), %rdi\n";
  out << "  movq $0, %rax\n";
  out << "  call printf@PLT\n";
  return 0;
}

// -----------------------------------------------------------------------------
// visit(Body) — procesa declaraciones y sentencias
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(Body *b) {
  for (auto dec : b->declarations)
    dec->accept(this);
  for (auto stm : b->StmList)
    stm->accept(this);
  return 0;
}

// -----------------------------------------------------------------------------
// visit(IfStm) — emite bloque if-then-else con etiquetas únicas
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(IfStm *stm) {
  int lbl = labelcont++;
  stm->condition->accept(this);
  out << "  cmpq $0, %rax\n";
  out << "  je else_" << lbl << "\n";
  stm->then->accept(this);
  out << "  jmp endif_" << lbl << "\n";
  out << "else_" << lbl << ":\n";
  if (stm->els)
    stm->els->accept(this);
  out << "endif_" << lbl << ":\n";
  return 0;
}

// -----------------------------------------------------------------------------
// visit(WhileStm) — emite bucle while con etiquetas únicas
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(WhileStm *stm) {
  int lbl = labelcont++;
  std::string oldBreak = currentBreakLabel;
  currentBreakLabel = "endwhile_" + std::to_string(lbl);
  out << "while_" << lbl << ":\n";
  stm->condition->accept(this);
  out << "  cmpq $0, %rax\n";
  out << "  je endwhile_" << lbl << "\n";
  stm->b->accept(this);
  out << "  jmp while_" << lbl << "\n";
  out << "endwhile_" << lbl << ":\n";
  currentBreakLabel = oldBreak;
  return 0;
}

// -----------------------------------------------------------------------------
// visit(ReturnStm) — salta al epílogo de la función
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(ReturnStm *stm) {
  stm->e->accept(this);
  out << "  jmp .end_" << nombreFuncion << "\n";
  return 0;
}

// -----------------------------------------------------------------------------
// visit(FunDec) — emite prólogo, cuerpo y epílogo de una función
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(FunDec *f) {
  entornoFuncion = true;
  memoria.clear();
  offset = -8;
  nombreFuncion = f->nombre;

  const std::vector<std::string> argRegs = {"%rdi", "%rsi", "%rdx",
                                            "%rcx", "%r8",  "%r9"};

  // ---- Prólogo ----
  out << "\n.globl " << f->nombre << "\n";
  out << f->nombre << ":\n";
  out << "  pushq %rbp\n";
  out << "  movq %rsp, %rbp\n";
  out << "  subq $" << funcontador[f->nombre] * 8 << ", %rsp\n";

  // Guardar parámetros en el frame local y registrar tipos struct
  int nParams = static_cast<int>(f->Pnombres.size());
  for (int i = 0; i < nParams; i++) {
    memoria[f->Pnombres[i]] = offset;
    out << "  movq " << argRegs[i] << ", " << offset << "(%rbp)\n";
    offset -= 8;
    // Registrar tipo struct del parámetro si aplica
    if (structDefs.count(f->Ptipos[i])) {
      varStructType[f->Pnombres[i]] = f->Ptipos[i];
    }
  }

  // Registrar variables locales declaradas
  for (auto dec : f->cuerpo->declarations)
    dec->accept(this);

  // ---- Cuerpo ----
  for (auto stm : f->cuerpo->StmList)
    stm->accept(this);

  // ---- Epílogo ----
  out << ".end_" << f->nombre << ":\n";
  out << "  leave\n";
  out << "  ret\n";

  entornoFuncion = false;
  return 0;
}

// -----------------------------------------------------------------------------
// visit(FcallExp) — emite una llamada a función
// Argumentos en registros según la ABI System V x86-64
// -----------------------------------------------------------------------------

int GenCodeVisitor::visit(FcallExp *exp) {
  const std::vector<std::string> argRegs = {"%rdi", "%rsi", "%rdx",
                                            "%rcx", "%r8",  "%r9"};
  int nArgs = static_cast<int>(exp->argumentos.size());
  for (int i = 0; i < nArgs; i++) {
    exp->argumentos[i]->accept(this);
    out << "  movq %rax, " << argRegs[i] << "\n";
  }
  out << "  call " << exp->nombre << "\n";
  return 0;
}

int GenCodeVisitor::visit(DoWhileStm *stm) {
  int lbl = labelcont++;
  std::string oldBreak = currentBreakLabel;
  currentBreakLabel = "endwhile_" + std::to_string(lbl);
  out << "dowhile_" << lbl << ":\n";
  stm->b->accept(this);
  stm->condition->accept(this);
  out << "  cmpq $0, %rax\n";
  out << "  jne dowhile_" << lbl << "\n";
  out << "endwhile_" << lbl << ":\n";
  currentBreakLabel = oldBreak;
  return 0;
}

int GenCodeVisitor::visit(BreakStm *stm) {
  if (currentBreakLabel.empty()) {
    std::cerr << "Error: break fuera de while, do-while o switch\n";
    exit(1);
  }
  out << "  jmp " << currentBreakLabel << "\n";
  return 0;
}

int GenCodeVisitor::visit(SwitchStm *stm) {
  int lbl = labelcont++;
  stm->e->accept(this);
  out << "  movq %rax, %r10\n";
  for (auto c : stm->cases) {
    out << "  movq $" << c->value << ", %rax\n";
    out << "  cmpq %rax, %r10\n";
    out << "  je case_" << lbl << "_" << c->value << "\n";
  }
  if (!stm->default_body.empty())
    out << "  jmp default_" << lbl << "\n";
  else
    out << "  jmp endswitch_" << lbl << "\n";
  std::string oldBreak = currentBreakLabel;
  currentBreakLabel = "endswitch_" + std::to_string(lbl);
  for (auto c : stm->cases) {
    out << "case_" << lbl << "_" << c->value << ":\n";
    for (auto s : c->body)
      s->accept(this);
    out << "  jmp endswitch_" << lbl << "\n";
  }
  if (!stm->default_body.empty()) {
    out << "default_" << lbl << ":\n";
    for (auto s : stm->default_body)
      s->accept(this);
  }
  currentBreakLabel = oldBreak;
  out << "endswitch_" << lbl << ":\n";
  return 0;
}

// Helper to resolve field index from struct type
static int resolveFieldIdx(
    const std::unordered_map<std::string, std::string> &varStructType,
    const std::unordered_map<std::string, std::vector<std::string>> &structDefs,
    const std::string &varName, const std::string &fieldName) {
  auto it = varStructType.find(varName);
  if (it != varStructType.end()) {
    auto it2 = structDefs.find(it->second);
    if (it2 != structDefs.end()) {
      auto &fields = it2->second;
      for (int i = 0; i < (int)fields.size(); ++i)
        if (fields[i] == fieldName) return i;
    }
  }
  return -1;
}

// Helper to get struct field count
static int getStructFieldCount(
    const std::unordered_map<std::string, std::vector<std::string>> &structDefs,
    const std::string &structType) {
  auto it = structDefs.find(structType);
  if (it != structDefs.end()) return (int)it->second.size();
  return 0;
}

int GenCodeVisitor::visit(StructExp *exp) {
  // Determine field count: use values.size() if non-empty, else lookup struct def
  int n = exp->values.size();
  if (n == 0) {
    n = getStructFieldCount(structDefs, exp->structType);
  }
  // malloc(n * 8) as immediate
  out << "  movq $" << (n * 8) << ", %rdi\n"
      << "  call malloc@PLT\n";
  // Pointer left in %rax; values stored by AssignStm post-processing
  return 0;
}

int GenCodeVisitor::visit(FieldAccessExp *exp) {
  int fieldIdx = resolveFieldIdx(varStructType, structDefs, exp->name, exp->field);
  if (fieldIdx < 0) {
    std::cerr << "Error: campo '" << exp->field << "' no encontrado en '" << exp->name << "'\n";
    exit(1);
  }
  // Use same index pattern as lists: movq $idx, %rdi; load base; deref
  out << "  movq $" << fieldIdx << ", %rdi\n";
  if (memoriaGlobal.count(exp->name))
    out << "  movq " << exp->name << "(%rip), %rax\n";
  else
    out << "  movq " << memoria[exp->name] << "(%rbp), %rax\n";
  out << "  movq (%rax, %rdi, 8), %rax\n";
  return 0;
}

int GenCodeVisitor::computeAddress(FieldAccessExp *exp) {
  int fieldIdx = resolveFieldIdx(varStructType, structDefs, exp->name, exp->field);
  if (fieldIdx < 0) {
    std::cerr << "Error: campo '" << exp->field << "' no encontrado en '" << exp->name << "'\n";
    exit(1);
  }
  // Same pattern as computeAddress(IndexExp) but with static index
  out << "  pushq %rax\n";
  out << "  movq $" << fieldIdx << ", %rax\n";
  out << "  movq %rax, %rdi\n";
  out << "  popq %rax\n";
  out << "  movq %rax, %rcx\n";
  if (memoriaGlobal.count(exp->name))
    out << "  movq " << exp->name << "(%rip), %rax\n";
  else
    out << "  movq " << memoria[exp->name] << "(%rbp), %rax\n";
  out << "  movq %rcx, (%rax, %rdi, 8)\n";
  return 0;
}

int GenCodeVisitor::visit(MatrixSizeExp *exp) {
  // Layout: flat array, rows*cols elements, NO header
  // Evaluate cols first, push; evaluate rows; pop cols; multiply; *8; malloc
  exp->cols->accept(this);
  out << "  pushq %rax\n";
  exp->rows->accept(this);
  out << "  popq %rcx\n";
  out << "  imulq %rcx, %rax\n";
  out << "  salq $3, %rax\n";
  out << "  movq %rax, %rdi\n";
  out << "  call malloc@PLT\n";
  // Store cols count for this variable (will be associated in AssignStm)
  // The cols value needs to be tracked per variable
  return 0;
}

int GenCodeVisitor::visit(MatrixIndexExp *exp) {
  // Static cols: row * cols + col, using (%rax, %rdi, 8) addressing
  // Get static cols from varMatrixCols map
  int cols = varMatrixCols.count(exp->name) ? varMatrixCols[exp->name] : 1;

  // 1) Evaluate row, push it
  exp->row->accept(this);
  out << "  pushq %rax\n";

  // 2) Evaluate col
  exp->col->accept(this);
  out << "  movq %rax, %rdi\n";   // %rdi = col

  // 3) Pop row
  out << "  popq %rax\n";          // %rax = row

  // 4) Compute offset = row * cols + col
  out << "  movq $" << cols << ", %rcx\n";
  out << "  imulq %rcx, %rax\n";   // rax = row * cols
  out << "  addq %rdi, %rax\n";    // rax = row * cols + col
  out << "  movq %rax, %rdi\n";    // rdi = flat index

  // 5) Load base and read
  if (memoriaGlobal.count(exp->name))
    out << "  movq " << exp->name << "(%rip), %rax\n";
  else
    out << "  movq " << memoria[exp->name] << "(%rbp), %rax\n";
  out << "  movq (%rax, %rdi, 8), %rax\n";
  return 0;
}

int GenCodeVisitor::computeAddress(MatrixIndexExp *exp) {
  int cols = varMatrixCols.count(exp->name) ? varMatrixCols[exp->name] : 1;

  out << "  pushq %rax\n"; // save value to assign

  // 1) Evaluate row, push
  exp->row->accept(this);
  out << "  pushq %rax\n";

  // 2) Evaluate col
  exp->col->accept(this);
  out << "  movq %rax, %rdi\n";

  // 3) Pop row
  out << "  popq %rax\n";

  // 4) Compute offset
  out << "  movq $" << cols << ", %rcx\n";
  out << "  imulq %rcx, %rax\n";
  out << "  addq %rdi, %rax\n";
  out << "  movq %rax, %rdi\n";

  // 5) Pop value, load base, store
  out << "  popq %rcx\n";
  if (memoriaGlobal.count(exp->name))
    out << "  movq " << exp->name << "(%rip), %rax\n";
  else
    out << "  movq " << memoria[exp->name] << "(%rbp), %rax\n";
  out << "  movq %rcx, (%rax, %rdi, 8)\n";
  return 0;
}

int GenCodeVisitor::visit(MatrixValsExp *exp) {
  // Same allocation as MatrixSizeExp: rows * cols * 8
  exp->cols->accept(this);
  out << "  pushq %rax\n";
  exp->rows->accept(this);
  out << "  popq %rcx\n";
  out << "  imulq %rcx, %rax\n";
  out << "  salq $3, %rax\n";
  out << "  movq %rax, %rdi\n";
  out << "  call malloc@PLT\n";
  // Values stored by AssignStm post-processing
  return 0;
}
