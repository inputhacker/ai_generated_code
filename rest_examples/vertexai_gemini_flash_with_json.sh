export PROJECT_ID=$PROJECT_ID
export MODEL_ID=$MODEL_ID
# MODEL_ID example : gemini-2.0-flash-001

curl -X POST   -H "Authorization: Bearer $(gcloud auth print-access-token)"   -H "Content-Type: application/json"   "https://aiplatform.googleapis.com/v1/projects/$PROJECT_ID/locations/global/publishers/google/models/$MODEL_ID:generateContent"   -d @request_example.json 
