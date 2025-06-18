export MODEL_ID=$MODEL_ID
#export MODEL_ID="gemini-2.0-flash-001"
export PROJECT_ID=$PROJECT_ID
#export PROJECT_ID={your project id}

curl   -X POST   -H "Authorization: Bearer $(gcloud auth application-default print-access-token)"   -H "Content-Type: application/json"   "https://us-central1-aiplatform.googleapis.com/v1/projects/${PROJECT_ID}/locations/us-central1/publishers/google/models/${MODEL_ID}:streamGenerateContent" -d \\
'{
       "contents": {
         "role": "user",
         "parts": [
           {
           "fileData": {
             "mimeType": "image/png",
             "fileUri": "gs://generativeai-downloads/images/scones.jpg"
             }
           },
           {
             "text": "Describe this picture."
           }
         ]
       }
     }'
