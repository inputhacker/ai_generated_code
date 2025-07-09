from fastapi import FastAPI, Request, HTTPException, Response
import uvicorn
import json
from typing import Dict, Any, List, Optional
from pydantic import BaseModel, Field

app = FastAPI()

# MCP 스키마 정의
class MCPTool(BaseModel):
    name: str
    description: str
    input_schema: Dict[str, Any]

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
        "name": "Simple MCP Calculator Server",
        "description": "간단한 계산 기능을 제공하는 MCP 서버",
        "tools_url": "/mcp/tools",
        "resources_url": "/mcp/resources",
        "prompts_url": "/mcp/prompts"
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

@app.post("/mcp/tools/add")
async def execute_add(request: ToolRequest):
    params = request.parameters
    if "a" not in params or "b" not in params:
        raise HTTPException(status_code=400, detail="Missing required parameters: a, b")
    
    try:
        result = float(params["a"]) + float(params["b"])
        return {"result": result}
    except (ValueError, TypeError):
        raise HTTPException(status_code=400, detail="Invalid parameters: a and b must be numbers")

@app.post("/mcp/tools/multiply")
async def execute_multiply(request: ToolRequest):
    params = request.parameters
    if "a" not in params or "b" not in params:
        raise HTTPException(status_code=400, detail="Missing required parameters: a, b")
    
    try:
        result = float(params["a"]) * float(params["b"])
        return {"result": result}
    except (ValueError, TypeError):
        raise HTTPException(status_code=400, detail="Invalid parameters: a and b must be numbers")

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
