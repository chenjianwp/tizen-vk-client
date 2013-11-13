/*
 * FileDownloader.cpp
 *
 *  Created on: Nov 12, 2013
 *      Author: wolong
 */


#include "FileDownloader.h"

#include "FIo.h"
#include "FSecCrypto.h"

#include "VKU.h"

using namespace Tizen::Base;
using namespace Tizen::Base::Utility;
using namespace Tizen::Net::Http;
using namespace Tizen::Ui;
using namespace Tizen::Io;
using namespace Tizen::Security::Crypto;
using namespace Tizen::Net::Http;
using namespace Tizen::Graphics;

class DownloadingImageData : public Object {
private:
	File *_file;
	ICacheEntry *_cacheEntry;
public:
	DownloadingImageData(ICacheEntry *cacheEntry) {
		_cacheEntry = cacheEntry;
		_file = new File();
		_file->Construct(cacheEntry->GetFile(), "w");
	}

	void Write(const ByteBuffer &buffer) {
		_file->Write(buffer);
	}

	void FinishRemove() {
		delete _file;
		File::Remove(_cacheEntry->GetFile());
		_cacheEntry->OnDownloadError();
	}

	void FinishFlush() {
		_file->Flush();
		delete _file;
		_cacheEntry->OnDownloadSuccess();
	}
};

FileDownloader::FileDownloader() {
	AppLog("constructor");
}

result FileDownloader::Construct() {
	result r;

	httpSession = new (std::nothrow) HttpSession();
	TryCatch(httpSession != null, r = E_FAILURE, "failed allocation http session");

	r = httpSession->Construct(NET_HTTP_SESSION_MODE_MULTIPLE_HOST, null, L"", null, NET_HTTP_COOKIE_FLAG_ALWAYS_MANUAL);
	TryCatch(r == E_SUCCESS, , "failed to create http session");

	r = httpSession->SetAutoRedirectionEnabled(true);
	TryCatch(r == E_SUCCESS, , "Failed to set the redirection automatically.");

	return E_SUCCESS;
	CATCH:
	AppLogException("Construct failed: %s", GetErrorMessage(r));
	return r;
}

FileDownloader::~FileDownloader() {

}

result FileDownloader::DownloadImage(ICacheEntry *cacheEntry) {
	result r;
	HttpTransaction *transaction;
	HttpRequest *request;

	transaction = httpSession->OpenTransactionN();
	TryCatch(GetLastResult() == E_SUCCESS, r = GetLastResult(), "Failed to open the HttpTransaction.");

	r = transaction->SetUserObject(new DownloadingImageData(cacheEntry));
	TryCatch(r == E_SUCCESS, , "Failed SetUserObject");

	r = transaction->AddHttpTransactionListener(*this);
	TryCatch(r == E_SUCCESS, , "Failed to add the HttpTransactionListener.");

	request = const_cast<HttpRequest *>(transaction->GetRequest());

	r = request->SetUri(cacheEntry->GetUrl());
	TryCatch(r == E_SUCCESS, , "Failed to set the uri.");

	r = request->SetMethod(NET_HTTP_METHOD_GET);
	TryCatch(r == E_SUCCESS, , "Failed to set the method.");

	r = transaction->Submit();
	TryCatch(r == E_SUCCESS, , "Failed to submit transaction");

	return E_SUCCESS;
	CATCH:
	AppLogException("VKUReqquest constructor is failed: %s", GetErrorMessage(r));
	return r;
}

void FileDownloader::OnTransactionReadyToRead(HttpSession& httpSession, HttpTransaction& httpTransaction, int availableBodyLen) {
	AppLog("OnTransactionReadyToRead");


	HttpResponse* httpResponse = httpTransaction.GetResponse();
	DownloadingImageData *downloadingData = static_cast<DownloadingImageData *>(httpTransaction.GetUserObject());

	if (httpResponse->GetHttpStatusCode() == HTTP_STATUS_OK) {
		ByteBuffer* buffer = httpResponse->ReadBodyN();
		downloadingData->Write(*buffer);
		delete buffer;
	}

	return;
}

void FileDownloader::OnTransactionAborted(HttpSession& httpSession, HttpTransaction& httpTransaction, result r) {
	AppLog("OnTransactionAborted(%s)", GetErrorMessage(r));

	DownloadingImageData *downloadingData = static_cast<DownloadingImageData *>(httpTransaction.GetUserObject());
	downloadingData->FinishRemove();
	delete downloadingData;
}

void FileDownloader::OnTransactionReadyToWrite(HttpSession& httpSession, HttpTransaction& httpTransaction, int recommendedChunkSize) {
	AppLog("OnTransactionReadyToWrite");
}

void FileDownloader::OnTransactionHeaderCompleted(HttpSession& httpSession, HttpTransaction& httpTransaction, int headerLen, bool authRequired) {
	AppLog("OnTransactionHeaderCompleted");
}

void FileDownloader::OnTransactionCompleted(HttpSession& httpSession, HttpTransaction& httpTransaction) {
	AppLog("OnTransactionCompleted");

	DownloadingImageData *downloadingData = static_cast<DownloadingImageData *>(httpTransaction.GetUserObject());
	downloadingData->FinishFlush();

	delete downloadingData;
}

void FileDownloader::OnTransactionCertVerificationRequiredN(HttpSession& httpSession, HttpTransaction& httpTransaction, Tizen::Base::String* pCert) {
	AppLog("OnTransactionCertVerificationRequiredN");
	httpTransaction.Resume();
	delete pCert;
}
