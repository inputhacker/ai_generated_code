from fastapi import FastAPI, Request, HTTPException, Response
from fastapi.responses import StreamingResponse
import uvicorn
import json
import asyncio
from typing import Dict, Any, List, Optional, AsyncGenerator
from pydantic import BaseModel, Field

app = FastAPI()

# MCP 스키마 정의
class MCPTool(BaseModel):
    name: str
    description: str
    input_schema: Dict[str, Any]
    streaming: bool = True  # 스트리밍 지원 표시

class MCPResource(BaseModel):
    name: str
    content: str
    content_type: str = "text/markdown"

class MCPPrompt(BaseModel):
    name: str
    content: str
    description: str

# MCP 서버 데이터 정의
mcp_tools = [
    {
        "name": "add",
        "description": "두 숫자를 더합니다.",
        "streaming": True,  # 스트리밍 지원 표시
        "input_schema": {
            "type": "object",
            "properties": {
                "a": {"type": "number", "description": "첫 번째 숫자"},
                "b": {"type": "number", "description": "두 번째 숫자"}
            },
            "required": ["a", "b"]
        }
    },
    {
        "name": "multiply",
        "description": "두 숫자를 곱합니다.",
        "streaming": True,  # 스트리밍 지원 표시
        "input_schema": {
            "type": "object",
            "properties": {
                "a": {"type": "number", "description": "첫 번째 숫자"},
                "b": {"type": "number", "description": "두 번째 숫자"}
            },
            "required": ["a", "b"]
        }
    }
]

mcp_resources = [
    {
        "name": "greeting",
        "content": "# 안녕하세요!\n\n이것은 MCP 서버의 첫 번째 리소스입니다.",
        "content_type": "text/markdown"
    },
    {
        "name": "instructions",
        "content": "# 사용 지침\n\n1. `add` 도구: 두 숫자를 더합니다.\n2. `multiply` 도구: 두 숫자를 곱합니다.",
        "content_type": "text/markdown"
    }
]

mcp_prompts = [
    {
        "name": "calculator",
        "content": "당신은 숫자 계산을 도와주는 친절한 계산기입니다. add와 multiply 도구를 사용하여 사용자의 수학 문제를 해결해 주세요.",
        "description": "수학 계산을 위한 프롬프트"
    }
]

# MCP 엔드포인트 구현
@app.get("/.well-known/mcp")
async def mcp_manifest():
    return {
        "schema_version": "mcp-0.1",
        "api_version": "0.1.0",
        "name": "Streaming MCP Calculator Server",
        "description": "스트리밍을 지원하는 계산 기능 MCP 서버",
        "tools_url": "/mcp/tools",
        "resources_url": "/mcp/resources",
        "prompts_url": "/mcp/prompts",
        "transport": {
            "type": "http",
            "streaming": True  # 서버가 스트리밍을 지원함을 명시
        }
    }

@app.get("/mcp/tools")
async def get_tools():
    return {
        "tools": mcp_tools
    }

@app.get("/mcp/resources")
async def get_resources():
    return {
        "resources": [{"name": r["name"], "url": f"/mcp/resources/{r['name']}"} for r in mcp_resources]
    }

@app.get("/mcp/resources/{resource_name}")
async def get_resource(resource_name: str):
    for resource in mcp_resources:
        if resource["name"] == resource_name:
            return Response(
                content=resource["content"],
                media_type=resource["content_type"]
            )
    raise HTTPException(status_code=404, detail=f"Resource '{resource_name}' not found")

@app.get("/mcp/prompts")
async def get_prompts():
    return {
        "prompts": mcp_prompts
    }

class ToolRequest(BaseModel):
    parameters: Dict[str, Any]
    stream: bool = False  # 클라이언트가 스트리밍을 요청하는지 여부

# 스트리밍 응답을 생성하는 함수
async def stream_add(a: float, b: float) -> AsyncGenerator[str, None]:
    # 계산 과정을 스트리밍으로 보여주기
    yield json.dumps({"status": "processing", "message": "입력값 검증 중..."}) + "\n"
    await asyncio.sleep(0.5)  # 실제 서비스에서는 필요에 따라 조정
    
    yield json.dumps({"status": "processing", "message": f"{a}와 {b}를 더하는 중..."}) + "\n"
    await asyncio.sleep(0.5)  # 실제 서비스에서는 필요에 따라 조정
    
    result = a + b
    yield json.dumps({"status": "complete", "result": result}) + "\n"

async def stream_multiply(a: float, b: float) -> AsyncGenerator[str, None]:
    # 계산 과정을 스트리밍으로 보여주기
    yield json.dumps({"status": "processing", "message": "입력값 검증 중..."}) + "\n"
    await asyncio.sleep(0.5)  # 실제 서비스에서는 필요에 따라 조정
    
    yield json.dumps({"status": "processing", "message": f"{a}와 {b}를 곱하는 중..."}) + "\n"
    await asyncio.sleep(0.5)  # 실제 서비스에서는 필요에 따라 조정
    
    result = a * b
    yield json.dumps({"status": "complete", "result": result}) + "\n"

@app.post("/mcp/tools/add")
async def execute_add(request: ToolRequest):
    params = request.parameters
    
    if "a" not in params or "b" not in params:
        raise HTTPException(status_code=400, detail="Missing required parameters: a, b")
    
    try:
        a = float(params["a"])
        b = float(params["b"])
        
        # 스트리밍 응답 요청 시
        if request.stream:
            return StreamingResponse(
                stream_add(a, b),
                media_type="application/x-ndjson"
            )
        
        # 스트리밍 아닌 경우 일반 응답
        result = a + b
        return {"result": result}
    except (ValueError, TypeError):
        raise HTTPException(status_code=400, detail="Invalid parameters: a and b must be numbers")

@app.post("/mcp/tools/multiply")
async def execute_multiply(request: ToolRequest):
    params = request.parameters
    
    if "a" not in params or "b" not in params:
        raise HTTPException(status_code=400, detail="Missing required parameters: a, b")
    
    try:
        a = float(params["a"])
        b = float(params["b"])
        
        # 스트리밍 응답 요청 시
        if request.stream:
            return StreamingResponse(
                stream_multiply(a, b),
                media_type="application/x-ndjson"
            )
        
        # 스트리밍 아닌 경우 일반 응답
        result = a * b
        return {"result": result}
    except (ValueError, TypeError):
        raise HTTPException(status_code=400, detail="Invalid parameters: a and b must be numbers")

if __name__ == "__main__":
    uvicorn.run(app, host="127.0.0.1", port=8000)
