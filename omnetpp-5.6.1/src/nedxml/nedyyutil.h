//==========================================================================
// nedyyutil.h  -
//
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2002-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __OMNETPP_NEDXML_NEDYYUTIL_H
#define __OMNETPP_NEDXML_NEDYYUTIL_H

#include "yyutil.h"
#include "nedelements.h"

namespace omnetpp {
namespace nedxml {
namespace nedyyutil {  // for now

ASTNode *createNedElementWithTag(ParseContext *np, int tagcode, ASTNode *parent=nullptr);
ASTNode *getOrCreateNedElementWithTag(ParseContext *np, int tagcode, ASTNode *parent);

PropertyElement *addProperty(ParseContext *np, ASTNode *node, const char *name);  // directly under the node
PropertyElement *addComponentProperty(ParseContext *np, ASTNode *node, const char *name); // into ParametersElement child of node

PropertyElement *storeSourceCode(ParseContext *np, ASTNode *node, YYLoc tokenpos);  // directly under the node
PropertyElement *storeComponentSourceCode(ParseContext *np, ASTNode *node, YYLoc tokenpos); // into ParametersElement child
PropertyElement *setIsNetworkProperty(ParseContext *np, ASTNode *node); // into ParametersElement child

void addComment(ParseContext *np, ASTNode *node, const char *locId, const char *comment, const char *defaultValue);
void storeFileComment(ParseContext *np, ASTNode *node);
void storeBannerComment(ParseContext *np, ASTNode *node, YYLoc tokenpos);
void storeRightComment(ParseContext *np, ASTNode *node, YYLoc tokenpos);
void storeTrailingComment(ParseContext *np, ASTNode *node, YYLoc tokenpos);
void storeBannerAndRightComments(ParseContext *np, ASTNode *node, YYLoc pos);
void storeBannerAndRightComments(ParseContext *np, ASTNode *node, YYLoc firstpos, YYLoc lastpos);
void storeInnerComments(ParseContext *np, ASTNode *node, YYLoc pos);

ParamElement *addParameter(ParseContext *np, ASTNode *params, YYLoc namepos);
ParamElement *addParameter(ParseContext *np, ASTNode *params, const char *name, YYLoc namepos);
GateElement *addGate(ParseContext *np, ASTNode *gates, YYLoc namepos);

void swapExpressionChildren(ASTNode *node, const char *attr1, const char *attr2);
void swapConnection(ASTNode *conn);

ExpressionElement *createExpression(ParseContext *np, ASTNode *expr);
OperatorElement *createOperator(ParseContext *np, const char *op, ASTNode *operand1, ASTNode *operand2=nullptr, ASTNode *operand3=nullptr);
FunctionElement *createFunction(ParseContext *np, const char *funcname, ASTNode *arg1=nullptr, ASTNode *arg2=nullptr, ASTNode *arg3=nullptr, ASTNode *arg4=nullptr, ASTNode *arg5=nullptr, ASTNode *arg6=nullptr, ASTNode *arg7=nullptr, ASTNode *arg8=nullptr, ASTNode *arg9=nullptr, ASTNode *arg10=nullptr);
IdentElement *createIdent(ParseContext *np, YYLoc parampos);
IdentElement *createIdent(ParseContext *np, YYLoc parampos, YYLoc modulepos, ASTNode *moduleindexoperand=nullptr);
LiteralElement *createPropertyValue(ParseContext *np, YYLoc textpos);
LiteralElement *createLiteral(ParseContext *np, int type, YYLoc valuepos, YYLoc textpos);
LiteralElement *createLiteral(ParseContext *np, int type, const char *value, const char *text);
LiteralElement *createStringLiteral(ParseContext *np, YYLoc textpos);
LiteralElement *createQuantityLiteral(ParseContext *np, YYLoc textpos);
ASTNode *prependMinusSign(ParseContext *np, ASTNode *node);

void addOptionalExpression(ParseContext *np, ASTNode *elem, const char *attrname, YYLoc exprpos, ASTNode *expr);
void addExpression(ParseContext *np, ASTNode *elem, const char *attrname, YYLoc exprpos, ASTNode *expr);

} // namespace nedyyutil
} // namespace nedxml
}  // namespace omnetpp


#endif

