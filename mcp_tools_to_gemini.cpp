//  File: mcp_to_gemini.cpp
//  g++ -std=c++17 -O2 mcp_to_gemini.cpp -o mcp_to_gemini

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*
 *  MCP tools/list → Gemini functionDeclarations 변환 함수
 *  - MCP 형식:  { "tools": [ { "name": "...", "description": "...",
 *                              "params":[ { "name":"..","type":"..", ... }, ... ] }, ... ] }
 *  - Gemini 형식: { "tools":[ { "functionDeclarations":[ { "name":"..",
 *                                                        "description":"..",
 *                                                        "parameters":{ "type":"object",
 *                                                                       "properties":{...},
 *                                                                       "required":[...] } }, ... ] } ] }
 */
json toGeminiTools(const json& mcpToolsList)
{
    json gemini;
    gemini["tools"] = json::array();

    // functionDeclarations 배열을 생성
    json functionDeclarations = json::array();

    if (!mcpToolsList.contains("tools") || !mcpToolsList["tools"].is_array())
        throw std::runtime_error("MCP JSON에 'tools' 배열이 없습니다.");

    for (const auto& tool : mcpToolsList["tools"])
    {
        json func;
        func["name"]        = tool.at("name");                       // 필수
        func["description"] = tool.value("description", "");         // 선택

        // parameters 객체 구성
        json parameters;
        parameters["type"] = "object";

        json properties = json::object();
        json required   = json::array();

        if (tool.contains("params") && tool["params"].is_array())
        {
            for (const auto& p : tool["params"])
            {
                json property;
                property["type"]        = p.value("type", "string");
                property["description"] = p.value("description", "");

                // enum(선택) 지원
                if (p.contains("enum") && p["enum"].is_array())
                    property["enum"] = p["enum"];

                // 속성 등록
                properties[p.at("name")] = property;

                // required 여부(기본 false)
                if (p.value("required", false))
                    required.push_back(p.at("name"));
            }
        }

        parameters["properties"] = properties;
        if (!required.empty())
            parameters["required"] = required;

        func["parameters"] = parameters;
        functionDeclarations.push_back(std::move(func));
    }

    // Gemini가 요구하는 wrapping
    gemini["tools"].push_back({ { "functionDeclarations", functionDeclarations } });
    return gemini;
}

int main()
{
    // 1) MCP 응답을 input.json에서 읽어오기
    std::ifstream ifs("input.json");
    if (!ifs)
    {
        std::cerr << "input.json 파일을 열 수 없습니다.\n";
        return 1;
    }

    try
    {
        json mcp;
        ifs >> mcp;

        // 2) 변환 수행
        json gemini = toGeminiTools(mcp);

        // 3) 변환 결과 출력 (pretty-print)
        std::cout << gemini.dump(2) << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << "변환 중 오류: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
