export MODEL_ID=$MODEL_ID
#export MODEL_ID="gemini-2.0-flash-001"
export PROJECT_ID=$PROJECT_ID
#export PROJECT_ID={your project id}

curl -X POST \
-H "Authorization: Bearer $(gcloud auth print-access-token)" \
-H "Content-Type: application/json" \
https://aiplatform.googleapis.com/v1/projects/${PROJECT_ID}/locations/global/publishers/google/models/${MODEL_ID}:generateContent -d '{
  "contents": {
    "role": "user",
    "parts": [
      {
      "fileData": {
        "mimeType": "video/mp4",
        "fileUri": "gs://cloud-samples-data/generative-ai/video/pixel8.mp4"
        }
      },
      {
        "text": "Provide a description of the video. The description should also contain anything important which people say in the video."
      }
    ]
  }
}'
