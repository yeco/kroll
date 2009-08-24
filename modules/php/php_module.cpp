/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <signal.h>
#include "php_module.h"
#include <Poco/Path.h>

#ifdef ZTS
void ***tsrm_ls;
#endif

namespace kroll
{
	KROLL_MODULE(PHPModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	PHPModule* PHPModule::instance_ = NULL;

	void PHPModule::Initialize()
	{
		PHPModule::instance_ = this;

		int argc = 1;
		char *argv[2] = { "php_kroll", NULL };
		php_embed_init(argc, argv PTSRMLS_CC);

		this->InitializeBinding();
		host->AddModuleProvider(this);
	}

	void PHPModule::Stop()
	{
		PHPModule::instance_ = NULL;

		SharedKObject global = this->host->GetGlobalObject();
		global->Set("PHP", Value::Undefined);
		this->binding->Set("evaluate", Value::Undefined);
		this->binding = NULL;
		PHPModule::instance_ = NULL;

		php_embed_shutdown(TSRMLS_C);
	}

	void PHPModule::InitializeBinding()
	{
		SharedKObject global = this->host->GetGlobalObject();
		this->binding = new StaticBoundObject();
		global->Set("PHP", Value::NewObject(this->binding));

		SharedKMethod evaluator = new PHPEvaluator();
		/**
		* @tiapi(method=True,name=PHP.evaluate,since=0.7) Evaluates a string as ruby code
		* @tiarg(for=PHP.evaluate,name=code,type=String) ruby script code
		* @tiarg(for=PHP.evaluate,name=scope,type=Object) global variable scope
		* @tiresult(for=PHP.evaluate,type=any) result of the evaluation
		*/
		this->binding->Set("evaluate", Value::NewMethod(evaluator));
	}

	const static std::string php_suffix = "module.php";

	bool PHPModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-php_suffix.length()) == php_suffix);
	}

	Module* PHPModule::CreateModule(std::string& path)
	{
		zend_first_try {
			std::string includeScript = "include '" + path + "'";
			zend_eval_string((char *)includeScript.c_str(), NULL, (char *) path.c_str() TSRMLS_CC);
		} zend_end_try();

		Poco::Path p(path);
		std::string basename = p.getBaseName();
		std::string name = basename.substr(0,basename.length()-php_suffix.length()+4);
		std::string moduledir = path.substr(0,path.length()-basename.length()-4);

		Logger *logger = Logger::Get("PHP");
		logger->Info("Loading PHP path=%s", path.c_str());

		return new PHPModuleInstance(host, path, moduledir, name);
	}
}
