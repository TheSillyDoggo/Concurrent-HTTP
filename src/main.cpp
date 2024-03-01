#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

void myCCHttpClientSend(CCHttpClient* ins, cocos2d::extension::CCHttpRequest* req) {
   //cocos2d::extension::CCHttpClient::send(req);

   log::info("url: {}", req->getUrl());

   if (std::string(req->getUrl()).starts_with("http://audio.ngfiles.com/") || std::string(req->getUrl()).starts_with("https://www.boomlings.com/database/getGJSongInfo.php") || std::string(req->getUrl()).starts_with("https://audio.ngfiles.com/") || std::string(req->getUrl()).starts_with("https://geometrydashfiles.b-cdn.net/"))
   {
      return ins->send(req);
   }

   req->retain();

   log::info("request body: {}\ntag: {}\nuser data: {}", req->getRequestData(), req->getTag(), req->getUserData());

   auto r = web::AsyncWebRequest();
   r.bodyRaw(req->getRequestData());

   auto v = req->getHeaders();

   for (size_t i = 0; i < v.size(); i++)
   {
      r.header(v[i]);
   }
   
   if (req->getRequestType() == CCHttpRequest::HttpRequestType::kHttpGet)
      r.method("GET");
   if (req->getRequestType() == CCHttpRequest::HttpRequestType::kHttpDelete)
      r.method("DELETE");
   if (req->getRequestType() == CCHttpRequest::HttpRequestType::kHttpPost)
      r.method("POST");
   if (req->getRequestType() == CCHttpRequest::HttpRequestType::kHttpPut)
      r.method("PUT");

   auto resp = r.fetch(req->getUrl());
   resp.text().then([req, resp](std::string const& catgirl) {
      log::info("success");

      //if (catgirl.starts_with("-"))
      log::info("heres the response :3 {}", catgirl);

      CCObject *pTarget = req->getTarget();
      SEL_HttpResponse pSelector = req->getSelector();

      if (pTarget && pSelector) 
      {
         CCHttpResponse *response = new CCHttpResponse(req);

         std::vector<char> charVector(catgirl.begin(), catgirl.end());

         response->setResponseData(&charVector);
         response->setSucceed(catgirl != "-1");
         response->setResponseCode(200);

         (pTarget->*pSelector)(CCHttpClient::getInstance(), response);

         delete response;
      }

      req->release();

   })
   .expect([req](std::string const& error) {
      log::info("failure: {}", error);

      CCHttpResponse *response = new CCHttpResponse(req);

      std::vector<char> charVector(error.begin(), error.end());

      response->setResponseData(&charVector);
      response->setSucceed(false);
      response->setResponseCode(-1);

      CCObject *pTarget = req->getTarget();
      SEL_HttpResponse pSelector = req->getSelector();
      (pTarget->*pSelector)(CCHttpClient::getInstance(), response);

      req->release();

      delete response;
   });

}

$execute {
    Mod::get()->hook(
        reinterpret_cast<void*>(
            geode::addresser::getNonVirtual(
                geode::modifier::Resolve<cocos2d::extension::CCHttpRequest*>::func(&CCHttpClient::send)
            )
        ),
        &myCCHttpClientSend,
        "cocos2d::extension::CCHttpClient::send",
        tulip::hook::TulipConvention::Thiscall
    );
}