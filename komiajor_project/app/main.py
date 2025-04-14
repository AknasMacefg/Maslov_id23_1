from fastapi import FastAPI, Depends
from app.api.users.router import router as router_users
from app.api.users.dependencies import get_token
from app.schemas.schemas import PathResult, PathIn, Graph
from typing import List, Tuple, Dict
import heapq

app = FastAPI()
@app.get("/")
def home_page():
    return {"message": "ИД23-1 МасловАН Коммивояжёр"}
  
app.include_router(router_users)

@app.post("/shortest-path", response_model=PathResult)
async def shortest_path(graph: PathIn, token: str = Depends(get_token)):
    path, total_distance = A_star(graph.graph, graph.start, graph.end)
    return PathResult(path=path, total_distance=total_distance)

#{ "graph": { "nodes": [1, 2, 3, 4], "edges": [[1, 2, 1], [2, 3, 2], [1, 4, 5], [3, 4, 1]] }, "start": 1, "end": 4 }
def A_star(graph: Graph, start: int, end: int) -> Tuple[List[int], float]:
    adjacency_list: Dict[int, List[Tuple[int, float]]] = {node: [] for node in graph.nodes}
    for u, v, weight in graph.edges:
        adjacency_list[u].append((v, weight))
        adjacency_list[v].append((u, weight))

    open_set = [(0, start, [])]  # (cost, current_node, path)
    visited = set()

    while open_set:
        cost, node, path = heapq.heappop(open_set)
        if node in visited:
            continue
        path = path + [node]
        if node == end:
            return path, cost
        visited.add(node)
        for neighbor, weight in adjacency_list[node]:
            if neighbor not in visited:
                heapq.heappush(open_set, (cost + weight, neighbor, path))

    return [], float('inf')