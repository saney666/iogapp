/*
 * FormatBase.h
 *
 *  Created on: May 18, 2018
 *      Author: jiasiang
 */

#ifndef FORMAT_FORMATBASE_H_
#define FORMAT_FORMATBASE_H_
#include "../class/TransactionParameter.h"
#include <string>

using namespace std;

class FormatBase {
public:
	virtual ~FormatBase();

	virtual void initialize(string)=0;
	virtual TransactionParameter* createCommand()=0;
	virtual bool processData(TransactionParameter*)=0;

	virtual TransactionParameter* prepareCommand()=0;
	virtual bool processCommand(TransactionParameter*)=0;
};

#endif /* FORMAT_FORMATBASE_H_ */
