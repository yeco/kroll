/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_ARG_LISTT_H_
#define _K_ARG_LISTT_H_

#include <vector>
#include <string>
#include <map>
#include "callback.h"

namespace kroll
{
	/**
	 * An argument list
	 *
	 * This class is only used for argument lists. For a list implementation to be
	 *  used as a value in the binding layer, take a look at KList and StaticBoundList.
	 */
	class KROLL_API ArgList
	{
	public:
		ArgList();
		ArgList(const ArgList&);
		~ArgList() {};

		bool Verify(const char* sig) const;
		void VerifyException(const char *name, const char* sig) const;

	public:
		void push_back(SharedValue value);
		size_t size() const;
		const SharedValue& at(size_t) const;
		const SharedValue& operator[](size_t) const;

	private:
		SharedPtr<std::vector<SharedValue> > args;

		static inline bool VerifyArg(SharedValue arg, const char* t);
		static inline std::vector<std::string>* ParseSigString(const char* sig);
		static SharedString GenerateSignature(const char* name, std::vector<std::string>* sig_vector);
		bool VerifyImpl(std::vector<std::string>* sig_vector) const;
	};

}

#endif