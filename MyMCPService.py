import asyncio
import base64
import json
import mimetypes
import os
from pathlib import Path
from typing import Dict, List, Optional, Union, Any

from fastapi import FastAPI, HTTPException, Request
from fastapi.responses import JSONResponse
from pydantic import BaseModel, Field

app = FastAPI(title="MCP Server Example")

# MCP 모델 정의
class MCPCalculatorInput(BaseModel):
    a: float = Field(..., description="첫 번째 숫자")
    b: float = Field(..., description="두 번째 숫자")

class MCPFilePathInput(BaseModel):
    path: str = Field(..., description="파일 경로")

class MCPTemplateInput(BaseModel):
    name: str = Field(..., description="템플릿에 표시할 이름")
    greeting: str = Field(..., description="인사말")
    message: str = Field(..., description="메시지")

class MCPPromptInput(BaseModel):
    system_prompt: str = Field(..., description="시스템 프롬프트")
    user_prompt: str = Field(..., description="사용자 프롬프트")

class ResourceData(BaseModel):
    type: str
     str
    mime_type: str

class ToolCallResult(BaseModel):
    content: Union[str, float, Dict[str, Any], List[Any]]

class MCPResponse(BaseModel):
    type: str = "mcp_completion"
    choices: List[Dict[str, Any]]
    
# MCP 서버 구현
@app.get("/v1/mcp/metadata")
async def get_metadata():
    """MCP 서버의 메타데이터를 반환합니다."""
    return {
        "type": "mcp_metadata",
        "capabilities": {
            "tools": {
                "add": {
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
                "subtract": {
                    "description": "첫 번째 숫자에서 두 번째 숫자를 뺍니다.",
                    "input_schema": {
                        "type": "object",
                        "properties": {
                            "a": {"type": "number", "description": "첫 번째 숫자"},
                            "b": {"type": "number", "description": "두 번째 숫자"}
                        },
                        "required": ["a", "b"]
                    }
                },
                "multiply": {
                    "description": "두 숫자를 곱합니다.",
                    "input_schema": {
                        "type": "object",
                        "properties": {
                            "a": {"type": "number", "description": "첫 번째 숫자"},
                            "b": {"type": "number", "description": "두 번째 숫자"}
                        },
                        "required": ["a", "b"]
                    }
                },
                "divide": {
                    "description": "첫 번째 숫자를 두 번째 숫자로 나눕니다.",
                    "input_schema": {
                        "type": "object",
                        "properties": {
                            "a": {"type": "number", "description": "첫 번째 숫자"},
                            "b": {"type": "number", "description": "두 번째 숫자 (0이 아니어야 함)"}
                        },
                        "required": ["a", "b"]
                    }
                }
            },
            "resources": {
                "read_text_file": {
                    "description": "지정된 경로의 텍스트 파일을 읽어 내용을 반환합니다.",
                    "input_schema": {
                        "type": "object",
                        "properties": {
                            "path": {"type": "string", "description": "텍스트 파일 경로"}
                        },
                        "required": ["path"]
                    }
                },
                "read_image_file": {
                    "description": "지정된 경로의 이미지 파일을 읽어 이미지 데이터를 반환합니다.",
                    "input_schema": {
                        "type": "object",
                        "properties": {
                            "path": {"type": "string", "description": "이미지 파일 경로"}
                        },
                        "required": ["path"]
                    }
                }
            },
            "templates": {
                "greeting_template": {
                    "description": "인사말 템플릿을 생성합니다.",
                    "input_schema": {
                        "type": "object",
                        "properties": {
                            "name": {"type": "string", "description": "템플릿에 표시할 이름"},
                            "greeting": {"type": "string", "description": "인사말"},
                            "message": {"type": "string", "description": "메시지"}
                        },
                        "required": ["name", "greeting", "message"]
                    }
                }
            },
            "prompts": {
                "custom_prompt": {
                    "description": "사용자 정의 프롬프트를 생성합니다.",
                    "input_schema": {
                        "type": "object",
                        "properties": {
                            "system_prompt": {"type": "string", "description": "시스템 프롬프트"},
                            "user_prompt": {"type": "string", "description": "사용자 프롬프트"}
                        },
                        "required": ["system_prompt", "user_prompt"]
                    }
                }
            }
        }
    }

# Tool API 구현
@app.post("/v1/mcp/tools/add")
async def add_numbers(data: MCPCalculatorInput):
    """두 숫자를 더합니다."""
    result = data.a + data.b
    return ToolCallResult(content=result)

@app.post("/v1/mcp/tools/subtract")
async def subtract_numbers( MCPCalculatorInput):
    """첫 번째 숫자에서 두 번째 숫자를 뺍니다."""
    result = data.a - data.b
    return ToolCallResult(content=result)

@app.post("/v1/mcp/tools/multiply")
async def multiply_numbers( MCPCalculatorInput):
    """두 숫자를 곱합니다."""
    result = data.a * data.b
    return ToolCallResult(content=result)

@app.post("/v1/mcp/tools/divide")
async def divide_numbers( MCPCalculatorInput):
    """첫 번째 숫자를 두 번째 숫자로 나눕니다."""
    if data.b == 0:
        raise HTTPException(status_code=400, detail="0으로 나눌 수 없습니다.")
    result = data.a / data.b
    return ToolCallResult(content=result)

# Resource API 구현
@app.post("/v1/mcp/resources/read_text_file")
async def read_text_file( MCPFilePathInput):
    """지정된 경로의 텍스트 파일을 읽어 내용을 반환합니다."""
    try:
        file_path = Path(data.path)
        if not file_path.exists():
            raise HTTPException(status_code=404, detail=f"파일을 찾을 수 없습니다: {data.path}")
        
        with open(file_path, "r", encoding="utf-8") as file:
            content = file.read()
            
        return ResourceData(
            type="text",
            data=content,
            mime_type="text/plain"
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"파일 읽기 오류: {str(e)}")

@app.post("/v1/mcp/resources/read_image_file")
async def read_image_file( MCPFilePathInput):
    """지정된 경로의 이미지 파일을 읽어 이미지 데이터를 반환합니다."""
    try:
        file_path = Path(data.path)
        if not file_path.exists():
            raise HTTPException(status_code=404, detail=f"파일을 찾을 수 없습니다: {data.path}")
        
        mime_type, _ = mimetypes.guess_type(file_path)
        if not mime_type or not mime_type.startswith('image/'):
            raise HTTPException(status_code=400, detail=f"지원되지 않는 이미지 형식입니다: {mime_type}")
        
        with open(file_path, "rb") as file:
            image_bytes = file.read()
            image_b64 = base64.b64encode(image_bytes).decode('utf-8')
            
        return ResourceData(
            type="image",
            data=image_b64,
            mime_type=mime_type
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"이미지 파일 읽기 오류: {str(e)}")

# Template API 구현
@app.post("/v1/mcp/templates/greeting_template")
async def greeting_template(data: MCPTemplateInput):
    """인사말 템플릿을 생성합니다."""
    template = f"""
    <div class="greeting-card">
        <h1>{data.greeting}, {data.name}!</h1>
        <p>{data.message}</p>
        <footer>MCP Template Example</footer>
    </div>
    """
    return MCPResponse(
        type="mcp_completion",
        choices=[{
            "index": 0,
            "message": {
                "role": "assistant",
                "content": template
            }
        }]
    )

# Prompt API 구현
@app.post("/v1/mcp/prompts/custom_prompt")
async def custom_prompt( MCPPromptInput):
    """사용자 정의 프롬프트를 생성합니다."""
    return MCPResponse(
        type="mcp_completion",
        choices=[{
            "index": 0,
            "message": {
                "role": "assistant",
                "content": f"프롬프트 처리 결과: 시스템 프롬프트 '{data.system_prompt}'에 대한 응답으로, '{data.user_prompt}'에 대한 답변을 생성합니다."
            }
        }]
    )

# 서버 실행
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
