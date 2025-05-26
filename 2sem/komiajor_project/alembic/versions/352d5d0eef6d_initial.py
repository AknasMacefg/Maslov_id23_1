"""initial

Revision ID: 352d5d0eef6d
Revises: 4adf2ed3d1ce
Create Date: 2025-05-26 00:37:17.806491

"""
from typing import Sequence, Union

from alembic import op
import sqlalchemy as sa


# revision identifiers, used by Alembic.
revision: str = '352d5d0eef6d'
down_revision: Union[str, None] = '4adf2ed3d1ce'
branch_labels: Union[str, Sequence[str], None] = None
depends_on: Union[str, Sequence[str], None] = None


def upgrade() -> None:
    """Upgrade schema."""
    pass


def downgrade() -> None:
    """Downgrade schema."""
    pass
