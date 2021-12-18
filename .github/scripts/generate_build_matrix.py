import json
from typing import Any

from generate_ci_workflows import LINUX_WORKFLOWS


matrix: Any = {"include": []}
for workflow in LINUX_WORKFLOWS:
    matrix["include"].append(
        {
            "build_environment": workflow.build_environment,
            "build_with_debug": workflow.build_with_debug,
            "docker_image_base": workflow.docker_image_base,
            "build_generates_artifacts": workflow.build_generates_artifacts,
        }
    )

print(json.dumps(matrix, indent=2))
print(f'::set-output name=matrix::{json.dumps(matrix)}')
