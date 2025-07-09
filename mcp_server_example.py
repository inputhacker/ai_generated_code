# requirements: fastapi, uvicorn

from fastapi import FastAPI, Request
from fastapi.responses import JSONResponse, StreamingResponse
import json

app = FastAPI()

# MCP 서버의 메타, tools, resources, prompt 정의
MCP_METADATA = {
    "id": "example-mcp-server",
    "version": "1.0.0",
    "name": "Example MCP Server",
    "description": "A sample MCP server supporting Streamable HTTP"
}

TOOLS = [
    {
        "name": "add",
        "description": "두 숫자를 더합니다.",
        "parameters": [
            {"name": "a", "type": "number"},
            {"name": "b", "type": "number"}
        ]
    },
    {
        "name": "multiply",
        "description": "두 숫자를 곱합니다.",
        "parameters": [
            {"name": "a", "type": "number"},
            {"name": "b", "type": "number"}
        ]
    }
]

RESOURCES = [
    {
        "name": "example_resource_1",
        "description": "예시 리소스 1"
    },
    {
        "name": "example_resource_2",
        "description": "예시 리소스 2"
    }
]

PROMPTS = [
    {
        "name": "example_prompt",
        "description": "예시 프롬프트",
        "content": "아래 지시에 따라 작업을 수행하세요."
    }
]

@app.post("/mcp")
async def mcp_endpoint(request: Request):
    body = await request.json()
    method = body.get("method")                # 예: "tools.add", "tools.multiply" 등
    params = body.get("params", {})

    # 메타 정보 요청
    if method == "meta":
        return JSONResponse({"meta": MCP_METADATA})

    # tool 목록 제공
    if method == "tools.list":
        return JSONResponse({"tools": TOOLS})

    # 리소스 목록 제공
    if method == "resources.list":
        return JSONResponse({"resources": RESOURCES})

    # 프롬프트 목록 제공
    if method == "prompts.list":
        return JSONResponse({"prompts": PROMPTS})

    # tool 실행
    if method == "tools.add":
        a, b = params.get("a"), params.get("b")
        result = a + b
        return JSONResponse({"result": result})
    if method == "tools.multiply":
        a, b = params.get("a"), params.get("b")
        result = a * b
        return JSONResponse({"result": result})

    # 기타/알 수 없는 method
    return JSONResponse({"error": "Unknown method"}, status_code=400)

if __name__ == "__main__":
    import uvicorn
    uvicorn.run("mcp_server:app", host="127.0.0.1", port=8001, reload=True)
