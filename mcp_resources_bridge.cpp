//  File : mcp_resources_bridge.cpp
//  빌드 : g++ -std=c++17 -O2 mcp_resources_bridge.cpp -o mcp_resources_bridge
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/* =============================================================================
 *  1) MCP resources/list  →  Gemini Resources-List
 *     MCP 가정 형식
 *       {
 *         "resources":[
 *            { "id":"r1", "type":"image", "name":"photo.jpg", …기타 필드들… },
 *            ...
 *         ]
 *       }
 *
 *     Gemini 변환 형식 예시
 *       {
 *         "resources":[
 *            {
 *              "resourceId":"r1",
 *              "resourceType":"image",
 *              "displayName":"photo.jpg",
 *              "metadata":{ ... MCP 의 기타 필드 ... }
 *            }, ...
 *         ]
 *       }
 * ===========================================================================*/
json toGeminiResourcesList(const json& mcpList)
{
    if (!mcpList.contains("resources") || !mcpList["resources"].is_array())
        throw std::runtime_error("MCP JSON 에 'resources' 배열이 없습니다.");

    json geminiResources = json::array();

    for (const auto& res : mcpList["resources"])
    {
        json gRes;
        gRes["resourceId"]   = res.at("id");          // 필수
        gRes["resourceType"] = res.value("type", ""); // 없으면 빈 문자열
        gRes["displayName"]  = res.value("name", "");

        /* 기타 필드를 metadata 로 묶어 전달 */
        json metadata = json::object();
        for (auto it = res.begin(); it != res.end(); ++it)
        {
            const std::string& key = it.key();
            if (key == "id" || key == "type" || key == "name")
                continue; // 이미 변환된 필드
            metadata[key] = it.value();
        }
        if (!metadata.empty())
            gRes["metadata"] = metadata;

        geminiResources.push_back(std::move(gRes));
    }

    return json{ { "resources", geminiResources } };
}

/* =============================================================================
 *  2) Gemini resources/read  →  MCP resources/read
 *     Gemini 가정 형식
 *       {
 *         "resourceRead":{
 *             "resourceId":"r1",
 *             "options":{
 *                   "includeBinary":true,
 *                   "fields":["size","content"]
 *             }
 *         }
 *       }
 *
 *     MCP 변환 형식 예시
 *       {
 *         "read":{
 *             "id":"r1",
 *             "options":{
 *                   "includeBinary":true,
 *                   "fields":["size","content"]
 *             }
 *         }
 *       }
 * ===========================================================================*/
json toMcpResourcesRead(const json& geminiRead)
{
    if (!geminiRead.contains("resourceRead") || !geminiRead["resourceRead"].is_object())
        throw std::runtime_error("Gemini JSON 에 'resourceRead' 오브젝트가 없습니다.");

    const json& r = geminiRead["resourceRead"];

    json mcp;
    mcp["read"]["id"] = r.at("resourceId");

    if (r.contains("options"))
        mcp["read"]["options"] = r["options"];

    return mcp;
}

/* -----------------------------------------------------------------------------
 *  테스트용 메인 함수
 *    - input_res_list.json  : MCP resources/list 샘플
 *    - input_res_read_gemini.json : Gemini resources/read 샘플
 * ---------------------------------------------------------------------------*/
int main()
{
    try
    {
        /* (1) MCP resources/list  →  Gemini 변환 ----------------------- */
        {
            std::ifstream ifs("input_res_list.json");
            if (!ifs) throw std::runtime_error("input_res_list.json 파일을 열 수 없습니다.");
            json mcpList; ifs >> mcpList;

            json geminiList = toGeminiResourcesList(mcpList);

            std::cout << "===== Gemini Resources-List =====\n";
            std::cout << geminiList.dump(2) << "\n\n";
        }

        /* (2) Gemini resources/read  →  MCP 변환 ----------------------- */
        {
            std::ifstream ifs("input_res_read_gemini.json");
            if (!ifs) throw std::runtime_error("input_res_read_gemini.json 파일을 열 수 없습니다.");
            json geminiRead; ifs >> geminiRead;

            json mcpRead = toMcpResourcesRead(geminiRead);

            std::cout << "===== MCP resources/read Request =====\n";
            std::cout << mcpRead.dump(2) << '\n';
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "오류: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
