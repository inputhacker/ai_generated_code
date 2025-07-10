from mcp.server.fastmcp import FastMCP, Router
from mcp.protocol import Response
from typing import List, Dict, Any, Optional
import base64
import os

# MCP 라우터 및 앱 초기화
router = Router()
app = FastMCP(router=router)

# 1. 계산 기능 구현 (Tools API)
@router.tools("/calculate/add")
async def add_numbers(a: float, b: float) -> Dict[str, float]:
    """두 숫자를 더합니다."""
    return {"result": a + b}

@router.tools("/calculate/subtract")
async def subtract_numbers(a: float, b: float) -> Dict[str, float]:
    """두 숫자를 뺍니다."""
    return {"result": a - b}

@router.tools("/calculate/multiply")
async def multiply_numbers(a: float, b: float) -> Dict[str, float]:
    """두 숫자를 곱합니다."""
    return {"result": a * b}

@router.tools("/calculate/divide")
async def divide_numbers(a: float, b: float) -> Dict[str, float]:
    """두 숫자를 나눕니다. 0으로 나눌 수 없습니다."""
    if b == 0:
        raise ValueError("0으로 나눌 수 없습니다.")
    return {"result": a / b}

# 3. 텍스트 파일 읽기 (Resources API)
@router.resources("/read/text")
async def read_text_file(path: str) -> Dict[str, str]:
    """지정된 경로의 텍스트 파일을 읽어서 반환합니다."""
    try:
        with open(path, 'r', encoding='utf-8') as file:
            content = file.read()
        return {"content": content}
    except FileNotFoundError:
        raise FileNotFoundError(f"파일을 찾을 수 없습니다: {path}")
    except Exception as e:
        raise Exception(f"파일 읽기 오류: {str(e)}")

# 4. 이미지 파일 읽기 (Resources API)
@router.resources("/read/image")
async def read_image_file(path: str) -> Dict[str, str]:
    """지정된 경로의 이미지 파일을 base64로 인코딩하여 반환합니다."""
    try:
        with open(path, 'rb') as file:
            image_data = file.read()
        
        # 이미지 파일 확장자 확인
        _, file_extension = os.path.splitext(path)
        file_extension = file_extension.lower().replace('.', '')
        
        # 이미지 MIME 타입 설정
        mime_types = {
            'png': 'image/png',
            'jpg': 'image/jpeg',
            'jpeg': 'image/jpeg',
            'gif': 'image/gif',
            'bmp': 'image/bmp'
        }
        mime_type = mime_types.get(file_extension, 'application/octet-stream')
        
        # Base64 인코딩
        base64_data = base64.b64encode(image_data).decode('utf-8')
        data_url = f"{mime_type};base64,{base64_data}"
        
        return {"image_data": data_url}
    except FileNotFoundError:
        raise FileNotFoundError(f"이미지 파일을 찾을 수 없습니다: {path}")
    except Exception as e:
        raise Exception(f"이미지 파일 읽기 오류: {str(e)}")

# 5. MCP 리소스 템플릿 API 예제
@router.resources("/template/user")
async def user_template(name: str, age: int, interests: List[str]) -> Dict[str, Any]:
    """사용자 정보를 기반으로 템플릿을 생성합니다."""
    # 템플릿 예제
    template = {
        "user_info": {
            "name": name,
            "age": age,
            "interests": interests,
            "profile_summary": f"{name}님은 {age}세이며, {', '.join(interests)}에 관심이 있습니다."
        },
        "generated_at": "현재 시간 기준으로 생성됨"
    }
    
    return template

# 6. MCP 프롬프트 형태의 API 예제
@router.prompt("/generate/greeting")
async def generate_greeting(prompt: str, name: Optional[str] = None, formal: bool = True) -> Response:
    """
    인사말을 생성하는 API입니다.
    
    Args:
        prompt (str): 추가적인 컨텍스트나 지침
        name (str, optional): 인사할 사람의 이름
        formal (bool, default=True): 공식적인 인사말을 사용할지 여부
    """
    greeting_prefix = "안녕하세요" if formal else "안녕"
    
    if name:
        base_greeting = f"{greeting_prefix}, {name}님! "
    else:
        base_greeting = f"{greeting_prefix}! "
    
    if prompt:
        full_greeting = base_greeting + prompt
    else:
        full_greeting = base_greeting + "만나서 반갑습니다."
    
    return Response(content=full_greeting)

# 2. MCP 서버에서 제공하는 모든 API에 대한 설명을 제공
@router.description()
async def get_api_description() -> Dict[str, Any]:
    """MCP 서버에서 제공하는 모든 API에 대한 설명을 반환합니다."""
    return {
        "name": "MCP Python Server",
        "version": "1.0.0",
        "description": "계산 기능, 파일 읽기, 이미지 처리 등을 제공하는 MCP 서버",
        "apis": {
            "tools": [
                {
                    "path": "/calculate/add",
                    "description": "두 숫자를 더합니다.",
                    "parameters": {"a": "float", "b": "float"},
                    "returns": {"result": "float"}
                },
                {
                    "path": "/calculate/subtract",
                    "description": "두 숫자를 뺍니다.",
                    "parameters": {"a": "float", "b": "float"},
                    "returns": {"result": "float"}
                },
                {
                    "path": "/calculate/multiply",
                    "description": "두 숫자를 곱합니다.",
                    "parameters": {"a": "float", "b": "float"},
                    "returns": {"result": "float"}
                },
                {
                    "path": "/calculate/divide",
                    "description": "두 숫자를 나눕니다. 0으로 나눌 수 없습니다.",
                    "parameters": {"a": "float", "b": "float"},
                    "returns": {"result": "float"}
                }
            ],
            "resources": [
                {
                    "path": "/read/text",
                    "description": "지정된 경로의 텍스트 파일을 읽어서 반환합니다.",
                    "parameters": {"path": "string (파일 경로)"},
                    "returns": {"content": "string"}
                },
                {
                    "path": "/read/image",
                    "description": "지정된 경로의 이미지 파일을 base64로 인코딩하여 반환합니다.",
                    "parameters": {"path": "string (이미지 파일 경로)"},
                    "returns": {"image_data": "string (base64 인코딩된 이미지)"}
                },
                {
                    "path": "/template/user",
                    "description": "사용자 정보를 기반으로 템플릿을 생성합니다.",
                    "parameters": {
                        "name": "string", 
                        "age": "integer", 
                        "interests": "string[]"
                    },
                    "returns": {"user_info": "object", "generated_at": "string"}
                }
            ],
            "prompt": [
                {
                    "path": "/generate/greeting",
                    "description": "인사말을 생성하는 API입니다.",
                    "parameters": {
                        "prompt": "string (추가 컨텍스트)", 
                        "name": "string (인사할 이름, 선택사항)", 
                        "formal": "boolean (공식적 인사말 여부)"
                    },
                    "returns": "string (생성된 인사말)"
                }
            ]
        }
    }

# STDIO 전송 계층으로 서버 실행
if __name__ == "__main__":
    app.run_stdio()
