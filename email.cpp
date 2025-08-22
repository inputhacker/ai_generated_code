// gcc smtp-mail.cpp -o smtp-mail -lcurl
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#define FROM    "<sender@example.com>"
#define TO      "<recipient@example.com>"
#define CC      "<cc@example.com>"

static const char *payload_text =
  "To: Recipient <recipient@example.com>\r\n"
  "Cc: CC Person <cc@example.com>\r\n"
  "From: Sender <sender@example.com>\r\n"
  "Subject: Test Subject\r\n"
  "\r\n" // 본문 시작
  "This is the body of the test email sent from C++ via libcurl.\r\n";

struct upload_status {
  size_t bytes_read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data = &payload_text[upload_ctx->bytes_read];
  size_t room = size * nmemb;
  size_t len = strlen(data);

  if (len > room)
    len = room;
  if (len > 0) {
    memcpy(ptr, data, len);
    upload_ctx->bytes_read += len;
    return len;
  }
  return 0; // 끝
}

int main(void) {
  CURL *curl = curl_easy_init();
  CURLcode res = CURLE_OK;
  if (curl) {
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx = { 0 };

    curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.example.com:587");
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);
    recipients = curl_slist_append(recipients, TO);
    recipients = curl_slist_append(recipients, CC);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    // 인증 예시
    curl_easy_setopt(curl, CURLOPT_USERNAME, "smtp_username");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "smtp_password");

    // TLS 사용 권장
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

    res = curl_easy_perform(curl);

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
  }
  return 0;
}
