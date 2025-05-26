"""initial

Revision ID: 0adfc8c3d052
Revises: 352d5d0eef6d
Create Date: 2025-05-26 01:24:22.539887

"""
from typing import Sequence, Union

from alembic import op
import sqlalchemy as sa


# revision identifiers, used by Alembic.
revision: str = '0adfc8c3d052'
down_revision: Union[str, None] = '352d5d0eef6d'
branch_labels: Union[str, Sequence[str], None] = None
depends_on: Union[str, Sequence[str], None] = None


def upgrade() -> None:
    """Upgrade schema."""
    pass


def downgrade() -> None:
    """Downgrade schema."""
    pass
