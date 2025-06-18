export PROJECT_ID={your google project id}
export MODEL_ID={gemini_model id}
# MODEL_ID example : gemini-2.0-flash-001

curl -X POST   -H "Authorization: Bearer $(gcloud auth print-access-token)"   -H "Content-Type: application/json"   "https://aiplatform.googleapis.com/v1/projects/$PROJECT_ID/locations/global/publishers/google/models/$MODEL_ID:generateContent"   -d '{
    "contents": {
      "role": "user",
      "parts": [
        {
          "text": "안녕하세요, Gemini! 오늘 날씨 어때?"
        }
      ]
    }
  }'
{
  "candidates": [
    {
      "content": {
        "role": "model",
        "parts": [
          {
            "text": "안녕하세요! 현재 위치를 알려주시거나, 날씨를 알고 싶은 지역을 알려주시면 날씨 정보를 알려드릴 수 있습니다. 😊\n"
          }
        ]
      },
      "finishReason": "STOP",
      "avgLogprobs": -0.11516979932785035
    }
  ],
  "usageMetadata": {
    "promptTokenCount": 13,
    "candidatesTokenCount": 40,
    "totalTokenCount": 53,
    "trafficType": "ON_DEMAND",
    "promptTokensDetails": [
      {
        "modality": "TEXT",
        "tokenCount": 13
      }
    ],
    "candidatesTokensDetails": [
      {
        "modality": "TEXT",
        "tokenCount": 40
      }
    ]
  },
  "modelVersion": "gemini-2.0-flash-001",
  "createTime": "2025-06-18T14:45:09.008876Z",
  "responseId": "9dBSaKxFwKTr3g_RjJXICg"
}
