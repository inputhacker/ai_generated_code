//=========================================================================
//  File : mcp_prompts_bridge.cpp
//  Build: g++ -std=c++17 -O2 mcp_prompts_bridge.cpp -o mcp_prompts_bridge
//  Note : nlohmann/json.hpp 헤더가 include 경로에 있어야 합니다.
//=========================================================================
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/* -----------------------------------------------------------------------
 * (1) MCP prompts/list  →  Gemini prompts list 변환
 *
 *   MCP 예시
 *   {
 *     "prompts":[
 *       { "id":"p1", "title":"Greeting", "description":"인사 프롬프트", "tags":["general"] },
 *       ...
 *     ]
 *   }
 *
 *   Gemini 요구 형식(예시)
 *   {
 *     "prompts":[
 *       {
 *         "promptId":"p1",
 *         "title":"Greeting",
 *         "description":"인사 프롬프트",
 *         "metadata":{ "tags":["general"] }
 *       }, ...
 *     ]
 *   }
 * ---------------------------------------------------------------------*/
json toGeminiPromptsList(const json& mcpList)
{
    if (!mcpList.contains("prompts") || !mcpList["prompts"].is_array())
        throw std::runtime_error("MCP JSON 에 'prompts' 배열이 없습니다.");

    json geminiArray = json::array();

    for (const auto& p : mcpList["prompts"])
    {
        json g;
        g["promptId"]    = p.at("id");
        g["title"]       = p.value("title", "");
        g["description"] = p.value("description", "");

        /* id/title/description 외 값은 metadata 로 넘김 */
        json metadata = json::object();
        for (auto it = p.begin(); it != p.end(); ++it)
        {
            const std::string& key = it.key();
            if (key == "id" || key == "title" || key == "description")
                continue;
            metadata[key] = it.value();
        }
        if (!metadata.empty())
            g["metadata"] = metadata;

        geminiArray.push_back(std::move(g));
    }

    return json{ { "prompts", geminiArray } };
}

/* -----------------------------------------------------------------------
 * (2) Gemini promptGet  →  MCP prompts/get 변환
 *
 *   Gemini 예시
 *   {
 *     "promptGet":{
 *       "promptId":"p1",
 *       "options":{
 *         "includeMetadata":true
 *       }
 *     }
 *   }
 *
 *   MCP 변환 결과
 *   {
 *     "get":{
 *       "id":"p1",
 *       "options":{
 *         "includeMetadata":true
 *       }
 *     }
 *   }
 * ---------------------------------------------------------------------*/
json toMcpPromptGet(const json& geminiGet)
{
    if (!geminiGet.contains("promptGet") || !geminiGet["promptGet"].is_object())
        throw std::runtime_error("Gemini JSON 에 'promptGet' 오브젝트가 없습니다.");

    const json& pg = geminiGet["promptGet"];

    json mcp;
    mcp["get"]["id"] = pg.at("promptId");

    if (pg.contains("options"))
        mcp["get"]["options"] = pg["options"];

    return mcp;
}

/* -----------------------------------------------------------------------
 *  테스트용 main
 *    - input_prompts_list.json      : MCP prompts/list 샘플
 *    - input_prompt_get_gemini.json : Gemini promptGet 샘플
 * ---------------------------------------------------------------------*/
int main()
{
    try
    {
        /* 1) prompts/list 변환 --------------------------------------- */
        {
            std::ifstream ifs("input_prompts_list.json");
            if (!ifs)
                throw std::runtime_error("input_prompts_list.json 파일을 열 수 없습니다.");
            json mcpList;  ifs >> mcpList;

            json geminiList = toGeminiPromptsList(mcpList);
            std::cout << "===== Gemini prompts list =====\n"
                      << geminiList.dump(2) << "\n\n";
        }

        /* 2) promptGet 변환 ------------------------------------------ */
        {
            std::ifstream ifs("input_prompt_get_gemini.json");
            if (!ifs)
                throw std::runtime_error("input_prompt_get_gemini.json 파일을 열 수 없습니다.");
            json geminiGet; ifs >> geminiGet;

            json mcpGet = toMcpPromptGet(geminiGet);
            std::cout << "===== MCP prompts/get request =====\n"
                      << mcpGet.dump(2) << '\n';
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "오류: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
